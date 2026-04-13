// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "HunterAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Hearing;
class UAISenseConfig_Sight;

UENUM(BlueprintType)
enum class EHunterState : uint8
{
    Idle,
    Patrol,
    Alert,
    Investigate,
    Chase
};

UCLASS()
class PRUEBA_API AHunterAIController : public AAIController
{
    GENERATED_BODY()

public:
    AHunterAIController();

protected:



    UPROPERTY(EditAnywhere, Category = "AI|Speed")
    float PatrolSpeed = 450.f;

    UPROPERTY(EditAnywhere, Category = "AI|Speed")
    float InvestigateSpeed = 550.f;

    UPROPERTY(EditAnywhere, Category = "AI|Speed")
    float ChaseSpeed = 900.f;


    UPROPERTY(EditAnywhere, Category = "AI|Alert")
    float AlertTime = 0.5f;

    FTimerHandle AlertTimer;

    UPROPERTY(EditAnywhere, Category = "AI|Capture")
    float RestartDelay = 2.0f;

    FTimerHandle RestartLevelTimer;

    void RestartCurrentLevel();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, Category = "AI|Capture")
    float CaptureDistance = 200.f;

    bool bPlayerCaptured = false;

    void CapturePlayer(APawn* PlayerPawn);
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
    float InvestigateTime = 3.0f;

    UPROPERTY(VisibleAnywhere, Category = "AI|State")
    EHunterState CurrentState = EHunterState::Idle;

    FTimerHandle PatrolTimer;
    FTimerHandle InvestigateTimer;

    FVector LastNoiseLocation = FVector::ZeroVector;

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    void SetState(EHunterState NewState);
    void StartPatrol();
    void EndInvestigateState();
    void EndAlertState();
    FVector GetRandomSearchLocation(const FVector& Origin);
};