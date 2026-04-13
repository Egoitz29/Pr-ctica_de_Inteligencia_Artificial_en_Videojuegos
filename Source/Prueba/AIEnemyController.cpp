#include "AIEnemyController.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Sight.h"

#include "EnvironmentQuery/EnvQuery.h"
#include "EnvironmentQuery/EnvQueryManager.h"

#include "NavigationSystem.h"
#include "DrawDebugHelpers.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

AAIEnemyController::AAIEnemyController()
{
    PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
    SetPerceptionComponent(*PerceptionComp);

    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = HearingRange;
    HearingConfig->SetMaxAge(StimulusMaxAge);
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;

    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 2000.f;
    SightConfig->LoseSightRadius = 2500.f;
    SightConfig->PeripheralVisionAngleDegrees = 70.f;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

    PerceptionComp->ConfigureSense(*HearingConfig);
    PerceptionComp->ConfigureSense(*SightConfig);

    PerceptionComp->SetDominantSense(HearingConfig->GetSenseImplementation());

    PrimaryActorTick.bCanEverTick = true;
}

void AAIEnemyController::BeginPlay()
{
    Super::BeginPlay();

    if (!PerceptionComp || !HearingConfig || !SightConfig)
    {
        return;
    }

    HearingConfig->HearingRange = HearingRange;
    HearingConfig->SetMaxAge(StimulusMaxAge);

    PerceptionComp->ConfigureSense(*HearingConfig);
    PerceptionComp->ConfigureSense(*SightConfig);
    PerceptionComp->SetDominantSense(HearingConfig->GetSenseImplementation());
    PerceptionComp->RequestStimuliListenerUpdate();

    PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &AAIEnemyController::OnTargetPerceptionUpdated);

    SetState(EEnemyState::Patrol);

    UE_LOG(LogTemp, Warning, TEXT("AIEnemyController BeginPlay"));
}

void AAIEnemyController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AAIEnemyController::SetState(EEnemyState NewState)
{
    CurrentState = NewState;

    GetWorld()->GetTimerManager().ClearTimer(PatrolTimer);
    GetWorld()->GetTimerManager().ClearTimer(InvestigateTimer);

    switch (CurrentState)
    {
    case EEnemyState::Idle:
        UE_LOG(LogTemp, Warning, TEXT("STATE: IDLE"));
        StopMovement();
        break;

    case EEnemyState::Patrol:
        UE_LOG(LogTemp, Warning, TEXT("STATE: PATROL"));
        StartPatrol();
        break;

    case EEnemyState::Investigate:
        UE_LOG(LogTemp, Warning, TEXT("STATE: INVESTIGATE"));
        MoveToLocation(LastNoiseLocation);

        GetWorld()->GetTimerManager().SetTimer(
            InvestigateTimer,
            this,
            &AAIEnemyController::EndInvestigateState,
            InvestigateTime,
            false
        );
        break;

    case EEnemyState::Flee:
        UE_LOG(LogTemp, Warning, TEXT("STATE: FLEE"));

        if (APawn* ControlledPawn = GetPawn())
        {
            if (ACharacter* EnemyCharacter = Cast<ACharacter>(ControlledPawn))
            {
                EnemyCharacter->GetCharacterMovement()->MaxWalkSpeed = 900.f;
            }
        }

        RunEscapeQuery();
        break;
    }
}

void AAIEnemyController::StartPatrol()
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
    {
        return;
    }

    const FVector PatrolPoint = GetRandomSearchLocation(ControlledPawn->GetActorLocation());
    MoveToLocation(PatrolPoint);

    GetWorld()->GetTimerManager().SetTimer(
        PatrolTimer,
        this,
        &AAIEnemyController::StartPatrol,
        5.0f,
        false
    );
}

void AAIEnemyController::EndInvestigateState()
{
    SetState(EEnemyState::Patrol);
}

FVector AAIEnemyController::GetRandomSearchLocation(const FVector& Origin)
{
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSys)
    {
        return Origin;
    }

    FNavLocation RandomPoint;
    const bool bFound = NavSys->GetRandomReachablePointInRadius(
        Origin,
        PatrolRadius,
        RandomPoint
    );

    if (bFound)
    {
        return RandomPoint.Location;
    }

    return Origin;
}

void AAIEnemyController::RunEscapeQuery()
{
    if (!EscapeQuery)
    {
        UE_LOG(LogTemp, Warning, TEXT("EscapeQuery no asignada"));
        return;
    }

    FEnvQueryRequest QueryRequest(EscapeQuery, GetPawn());
    QueryRequest.Execute(
        EEnvQueryRunMode::SingleResult,
        this,
        &AAIEnemyController::OnEscapeQueryFinished
    );
}

void AAIEnemyController::OnEscapeQueryFinished(TSharedPtr<FEnvQueryResult> Result)
{
    if (!Result.IsValid() || !Result->IsSuccessful())
    {
        return;
    }

    const FVector EscapeLocation = Result->GetItemAsLocation(0);

    DrawDebugSphere(GetWorld(), EscapeLocation, 80.f, 12, FColor::Red, false, 2.0f);

    MoveToLocation(EscapeLocation);
}

void AAIEnemyController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor)
    {
        return;
    }

    const FAISenseID SightID = UAISense::GetSenseID<UAISense_Sight>();
    const FAISenseID HearingID = UAISense::GetSenseID<UAISense_Hearing>();

    if (!Stimulus.WasSuccessfullySensed())
    {
        return;
    }

    if (Stimulus.Type == HearingID)
    {
        LastNoiseLocation = Stimulus.StimulusLocation;

        UE_LOG(LogTemp, Warning, TEXT("NOISE HEARD"));

        DrawDebugSphere(GetWorld(), LastNoiseLocation, 60.f, 12, FColor::Yellow, false, 2.f);

        SetState(EEnemyState::Investigate);
        return;
    }

    if (Stimulus.Type == SightID)
    {
        UE_LOG(LogTemp, Warning, TEXT("PLAYER SEEN -> FLEE"));
        SetState(EEnemyState::Flee);
        return;
    }
}