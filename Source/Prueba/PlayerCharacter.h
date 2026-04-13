#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

class AEnemyCharacter;

UCLASS()
class PRUEBA_API APlayerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    APlayerCharacter();

    float NoiseInterval = 0.6f;
    float LastNoiseTime = -1000.f;
    float MinMovementSpeedForNoise = 10.f;

    UPROPERTY(BlueprintReadOnly, Category = "UI")
    bool bIsInStealthUI = false;

protected:
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable)
    void EmitNoiseTest();

private:
    void MoveForward(float Value);
    void MoveRight(float Value);

    void Turn(float Value);
    void LookUp(float Value);

    void StartStealth();
    void StopStealth();

    void CheckTouchWinCondition();

    bool bIsInStealth = false;
    bool bTouchWinTriggered = false;

    float NormalWalkSpeed = 900.f;
    float StealthWalkSpeed = 250.f;

    float NormalNoiseInterval = 0.6f;
    float StealthNoiseInterval = 1.5f;

    float TouchWinDistance = 120.f;
};