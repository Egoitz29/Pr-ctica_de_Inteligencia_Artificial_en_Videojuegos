#include "PruebaGameMode.h"
#include "PlayerCharacter.h"
#include "EnemyCharacter.h"
#include "AIEnemyController.h"
#include "HunterAIController.h"

#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "TimerManager.h"

APruebaGameMode::APruebaGameMode()
{
    DefaultPawnClass = APlayerCharacter::StaticClass();
}

void APruebaGameMode::BeginPlay()
{
    Super::BeginPlay();

    SecondsRemainingUI = FMath::CeilToInt(SurvivalTimeToWin);
    EndMessage = TEXT("");

    GetWorldTimerManager().SetTimer(
        WinTimerHandle,
        this,
        &APruebaGameMode::HandleWin,
        SurvivalTimeToWin,
        false
    );

    GetWorldTimerManager().SetTimer(
        CountdownDisplayTimerHandle,
        this,
        &APruebaGameMode::UpdateCountdownDisplay,
        0.1f,
        true
    );
}

void APruebaGameMode::StopAllEnemies()
{
    TArray<AActor*> EnemyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), EnemyActors);

    for (AActor* Actor : EnemyActors)
    {
        ACharacter* Character = Cast<ACharacter>(Actor);
        if (!Character)
        {
            continue;
        }

        AController* Controller = Character->GetController();
        if (!Controller)
        {
            continue;
        }

        if (Cast<AAIEnemyController>(Controller) || Cast<AHunterAIController>(Controller))
        {
            Controller->StopMovement();
            Character->GetCharacterMovement()->DisableMovement();
        }
    }
}

void APruebaGameMode::TriggerWinByTouchingPrey()
{
    if (bGameFinished)
    {
        return;
    }

    GetWorldTimerManager().ClearTimer(WinTimerHandle);
    GetWorldTimerManager().ClearTimer(CountdownDisplayTimerHandle);

    bGameFinished = true;
    EndMessage = TEXT("YOU WIN - PREY TOUCHED");

    UE_LOG(LogTemp, Warning, TEXT("YOU WIN - PREY TOUCHED"));

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            3.0f,
            FColor::Green,
            TEXT("YOU WIN - PREY TOUCHED")
        );
    }

    StopAllEnemies();

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC)
    {
        APawn* PlayerPawn = PC->GetPawn();
        if (PlayerPawn)
        {
            ACharacter* PlayerCharacter = Cast<ACharacter>(PlayerPawn);
            if (PlayerCharacter)
            {
                PlayerCharacter->GetCharacterMovement()->DisableMovement();
            }
        }

        PC->DisableInput(PC);
    }

    GetWorldTimerManager().SetTimer(
        RestartTimerHandle,
        this,
        &APruebaGameMode::RestartCurrentLevel,
        RestartDelayAfterWin,
        false
    );
}

void APruebaGameMode::HandleWin()
{
    if (bGameFinished)
    {
        return;
    }

    bGameFinished = true;
    EndMessage = TEXT("YOU WIN");

    UE_LOG(LogTemp, Warning, TEXT("YOU WIN"));

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            3.0f,
            FColor::Green,
            TEXT("YOU WIN")
        );
    }

    StopAllEnemies();

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC)
    {
        APawn* PlayerPawn = PC->GetPawn();
        if (PlayerPawn)
        {
            ACharacter* PlayerCharacter = Cast<ACharacter>(PlayerPawn);
            if (PlayerCharacter)
            {
                PlayerCharacter->GetCharacterMovement()->DisableMovement();
            }
        }

        PC->DisableInput(PC);
    }

    GetWorldTimerManager().SetTimer(
        RestartTimerHandle,
        this,
        &APruebaGameMode::RestartCurrentLevel,
        RestartDelayAfterWin,
        false
    );
}

void APruebaGameMode::UpdateCountdownDisplay()
{
    if (bGameFinished)
    {
        GetWorldTimerManager().ClearTimer(CountdownDisplayTimerHandle);
        return;
    }

    if (!GetWorldTimerManager().IsTimerActive(WinTimerHandle))
    {
        return;
    }

    const float TimeRemaining = FMath::Max(0.0f, GetWorldTimerManager().GetTimerRemaining(WinTimerHandle));
    const int32 SecondsRemaining = FMath::CeilToInt(TimeRemaining);

    SecondsRemainingUI = SecondsRemaining;

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            1,
            0.15f,
            FColor::Yellow,
            FString::Printf(TEXT("Tiempo para ganar: %d"), SecondsRemaining)
        );
    }
}

void APruebaGameMode::RestartCurrentLevel()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    FString LevelName = World->GetName();
    UGameplayStatics::OpenLevel(World, FName(*LevelName));
}