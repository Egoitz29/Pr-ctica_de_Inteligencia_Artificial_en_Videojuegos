#include "HunterAIController.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Sight.h"

#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "PruebaGameMode.h"

#include "NavigationSystem.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Pawn.h"


AHunterAIController::AHunterAIController()
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
    PerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());

    PrimaryActorTick.bCanEverTick = true;
}

void AHunterAIController::BeginPlay()
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
    PerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
    PerceptionComp->RequestStimuliListenerUpdate();

    PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &AHunterAIController::OnTargetPerceptionUpdated);

    SetState(EHunterState::Patrol);

    UE_LOG(LogTemp, Warning, TEXT("HunterAIController BeginPlay"));
}

void AHunterAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (CurrentState == EHunterState::Chase && !bPlayerCaptured)
    {
        APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
        APawn* ControlledPawn = GetPawn();

        if (PlayerPawn && ControlledPawn)
        {
            MoveToActor(PlayerPawn, 100.f);

            const float DistanceToPlayer = FVector::Dist(
                ControlledPawn->GetActorLocation(),
                PlayerPawn->GetActorLocation()
            );

            if (DistanceToPlayer <= CaptureDistance)
            {
                CapturePlayer(PlayerPawn);
            }
        }
    }
}

void AHunterAIController::SetState(EHunterState NewState)
{
    CurrentState = NewState;
    GetWorld()->GetTimerManager().ClearTimer(AlertTimer);
    GetWorld()->GetTimerManager().ClearTimer(PatrolTimer);
    GetWorld()->GetTimerManager().ClearTimer(InvestigateTimer);


    switch (CurrentState)
    {

    case EHunterState::Alert:
        UE_LOG(LogTemp, Warning, TEXT("HUNTER STATE: ALERT"));

        StopMovement();

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                5,
                1.0f,
                FColor::Orange,
                TEXT("HUNTER ALERT")
            );
        }

        GetWorld()->GetTimerManager().SetTimer(
            AlertTimer,
            this,
            &AHunterAIController::EndAlertState,
            AlertTime,
            false
        );
        break;
    case EHunterState::Idle:
        UE_LOG(LogTemp, Warning, TEXT("HUNTER STATE: IDLE"));
        StopMovement();
        break;

    case EHunterState::Patrol:
        UE_LOG(LogTemp, Warning, TEXT("HUNTER STATE: PATROL"));

        if (ACharacter* HunterCharacter = Cast<ACharacter>(GetPawn()))
        {
            HunterCharacter->GetCharacterMovement()->MaxWalkSpeed = 450.f;
        }

        StartPatrol();
        break;

    case EHunterState::Investigate:
        UE_LOG(LogTemp, Warning, TEXT("HUNTER STATE: INVESTIGATE"));

        if (ACharacter* HunterCharacter = Cast<ACharacter>(GetPawn()))
        {
            HunterCharacter->GetCharacterMovement()->MaxWalkSpeed = 550.f;
        }

        MoveToLocation(LastNoiseLocation);

        GetWorld()->GetTimerManager().SetTimer(
            InvestigateTimer,
            this,
            &AHunterAIController::EndInvestigateState,
            InvestigateTime,
            false
        );
        break;

    case EHunterState::Chase:
        UE_LOG(LogTemp, Warning, TEXT("HUNTER STATE: CHASE"));

        if (ACharacter* HunterCharacter = Cast<ACharacter>(GetPawn()))
        {
            HunterCharacter->GetCharacterMovement()->MaxWalkSpeed = 900.f;
        }
        break;
    }
}
void AHunterAIController::CapturePlayer(APawn* PlayerPawn)
{

    if (!PlayerPawn || bPlayerCaptured)
    {
        return;
    }

    bPlayerCaptured = true;


    APruebaGameMode* GM = Cast<APruebaGameMode>(GetWorld()->GetAuthGameMode());
    if (GM)
    {
        GM->EndMessage = TEXT("PLAYER CAPTURED");
    }

    UE_LOG(LogTemp, Warning, TEXT("PLAYER CAPTURED"));

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            2.0f,
            FColor::Red,
            TEXT("PLAYER CAPTURED")
        );
    }

    ACharacter* PlayerCharacter = Cast<ACharacter>(PlayerPawn);
    if (PlayerCharacter)
    {
        PlayerCharacter->GetCharacterMovement()->DisableMovement();
    }

    APlayerController* PC = Cast<APlayerController>(PlayerPawn->GetController());
    if (PC)
    {
        PC->DisableInput(PC);
    }

    StopMovement();

    GetWorld()->GetTimerManager().SetTimer(
        RestartLevelTimer,
        this,
        &AHunterAIController::RestartCurrentLevel,
        RestartDelay,
        false
    );
}

void AHunterAIController::EndAlertState()
{
    SetState(EHunterState::Investigate);
}

void AHunterAIController::RestartCurrentLevel()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    FString LevelName = World->GetName();
    UGameplayStatics::OpenLevel(World, FName(*LevelName));
}

void AHunterAIController::StartPatrol()
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
        &AHunterAIController::StartPatrol,
        5.0f,
        false
    );
}

void AHunterAIController::EndInvestigateState()
{
    SetState(EHunterState::Patrol);
}

FVector AHunterAIController::GetRandomSearchLocation(const FVector& Origin)
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

void AHunterAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor)
    {
        return;
    }

    const FAISenseID SightID = UAISense::GetSenseID<UAISense_Sight>();
    const FAISenseID HearingID = UAISense::GetSenseID<UAISense_Hearing>();

    if (!Stimulus.WasSuccessfullySensed())
    {
        if (Stimulus.Type == SightID && CurrentState == EHunterState::Chase)
        {
            UE_LOG(LogTemp, Warning, TEXT("HUNTER LOST PLAYER"));
            SetState(EHunterState::Patrol);
        }

        return;
    }

    if (Stimulus.Type == HearingID)
    {
        LastNoiseLocation = Stimulus.StimulusLocation;

        UE_LOG(LogTemp, Warning, TEXT("HUNTER HEARD NOISE"));

        DrawDebugSphere(GetWorld(), LastNoiseLocation, 60.f, 12, FColor::Yellow, false, 2.f);

        if (CurrentState != EHunterState::Chase)
        {
            SetState(EHunterState::Alert);
        }

        return;
    }

    if (Stimulus.Type == SightID)
    {
        UE_LOG(LogTemp, Warning, TEXT("HUNTER SAW PLAYER"));
        SetState(EHunterState::Chase);
        return;
    }
}