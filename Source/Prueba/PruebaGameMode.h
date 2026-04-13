#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PruebaGameMode.generated.h"

UCLASS()
class PRUEBA_API APruebaGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    APruebaGameMode();

    UPROPERTY(BlueprintReadOnly, Category = "UI")
    int32 SecondsRemainingUI = 0;

    UPROPERTY(BlueprintReadOnly, Category = "UI")
    FString EndMessage = TEXT("");

    UFUNCTION(BlueprintCallable)
    void TriggerWinByTouchingPrey();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, Category = "Game")
    float SurvivalTimeToWin = 60.0f;

    UPROPERTY(EditAnywhere, Category = "Game")
    float RestartDelayAfterWin = 3.0f;

    FTimerHandle WinTimerHandle;
    FTimerHandle RestartTimerHandle;

    bool bGameFinished = false;

    FTimerHandle CountdownDisplayTimerHandle;

    void UpdateCountdownDisplay();
    void HandleWin();
    void RestartCurrentLevel();
    void StopAllEnemies();
};