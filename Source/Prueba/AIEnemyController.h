#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "AIEnemyController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Hearing;
class UAISenseConfig_Sight;
class UEnvQuery;

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
    Idle,
    Patrol,
    Investigate,
    Flee
};

UCLASS()
class PRUEBA_API AAIEnemyController : public AAIController
{
    GENERATED_BODY()

public:
    AAIEnemyController();

    EEnemyState GetCurrentState() const { return CurrentState; }

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, Category = "AI")
    TObjectPtr<UAIPerceptionComponent> PerceptionComp;

    UPROPERTY()
    TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

    UPROPERTY()
    TObjectPtr<UAISenseConfig_Sight> SightConfig;

    UPROPERTY(EditAnywhere, Category = "AI|Hearing")
    float HearingRange = 1500.f;

    UPROPERTY(EditAnywhere, Category = "AI|Hearing")
    float StimulusMaxAge = 4.f;

    UPROPERTY(EditAnywhere, Category = "AI|Patrol")
    float PatrolRadius = 800.f;

    UPROPERTY(EditAnywhere, Category = "AI|Investigate")
    float InvestigateTime = 4.0f;

    UPROPERTY(EditAnywhere, Category = "AI|Escape")
    TObjectPtr<UEnvQuery> EscapeQuery;

    UPROPERTY(VisibleAnywhere, Category = "AI|State")
    EEnemyState CurrentState = EEnemyState::Idle;

    FTimerHandle PatrolTimer;
    FTimerHandle InvestigateTimer;

    FVector LastNoiseLocation = FVector::ZeroVector;

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    void SetState(EEnemyState NewState);
    void StartPatrol();
    void EndInvestigateState();

    FVector GetRandomSearchLocation(const FVector& Origin);

    void RunEscapeQuery();
    void OnEscapeQueryFinished(TSharedPtr<FEnvQueryResult> Result);
};