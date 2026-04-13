#include "EnemyCharacter.h"
#include "AIEnemyController.h"
#include "PlayerCharacter.h"
#include "PruebaGameMode.h"

#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

AEnemyCharacter::AEnemyCharacter()
{
    AIControllerClass = AAIEnemyController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (GetCapsuleComponent())
    {
        GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(
            this,
            &AEnemyCharacter::OnEnemyBeginOverlap
        );
    }
}

void AEnemyCharacter::OnEnemyBeginOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult
)
{
    if (!OtherActor)
    {
        return;
    }

    APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
    if (!Player)
    {
        return;
    }

    APruebaGameMode* GM = Cast<APruebaGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
    if (!GM)
    {
        return;
    }

    GM->TriggerWinByTouchingPrey();
}