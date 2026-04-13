#include "PlayerCharacter.h"

#include "Components/InputComponent.h"
#include "Perception/AISense_Hearing.h"
#include "DrawDebugHelpers.h"

#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "EnemyCharacter.h"
#include "PruebaGameMode.h"

APlayerCharacter::APlayerCharacter()
{
    GetCharacterMovement()->MaxWalkSpeed = NormalWalkSpeed;

    bUseControllerRotationYaw = false;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    PrimaryActorTick.bCanEverTick = true;
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    check(PlayerInputComponent);

    PlayerInputComponent->BindAction(TEXT("EmitNoise"), IE_Pressed, this, &APlayerCharacter::EmitNoiseTest);
    PlayerInputComponent->BindAction(TEXT("Stealth"), IE_Pressed, this, &APlayerCharacter::StartStealth);
    PlayerInputComponent->BindAction(TEXT("Stealth"), IE_Released, this, &APlayerCharacter::StopStealth);

    PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &APlayerCharacter::MoveForward);
    PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &APlayerCharacter::MoveRight);

    PlayerInputComponent->BindAxis(TEXT("Turn"), this, &APlayerCharacter::AddControllerYawInput);
    PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &APlayerCharacter::AddControllerPitchInput);
}

void APlayerCharacter::MoveForward(float Value)
{
    if (Controller && Value != 0.f)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(Direction, Value);
    }
}

void APlayerCharacter::MoveRight(float Value)
{
    if (Controller && Value != 0.f)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(Direction, Value);
    }
}

void APlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    const float Speed = GetVelocity().Size();

    if (Speed > MinMovementSpeedForNoise)
    {
        const float TimeNow = GetWorld()->GetTimeSeconds();

        if (TimeNow - LastNoiseTime > NoiseInterval)
        {
            EmitNoiseTest();
            LastNoiseTime = TimeNow;
        }
    }

    CheckTouchWinCondition();
}

void APlayerCharacter::CheckTouchWinCondition()
{
    if (bTouchWinTriggered)
    {
        return;
    }

    TArray<AActor*> FoundEnemies;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemyCharacter::StaticClass(), FoundEnemies);

    for (AActor* Actor : FoundEnemies)
    {
        AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(Actor);
        if (!Enemy)
        {
            continue;
        }

        const float Distance = FVector::Dist(GetActorLocation(), Enemy->GetActorLocation());

        if (Distance <= TouchWinDistance)
        {
            bTouchWinTriggered = true;

            APruebaGameMode* GM = Cast<APruebaGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
            if (GM)
            {
                GM->TriggerWinByTouchingPrey();
            }

            return;
        }
    }
}

void APlayerCharacter::EmitNoiseTest()
{
    UE_LOG(LogTemp, Warning, TEXT("EmitNoise emitido"));

    const FVector NoiseLocation = GetActorLocation();

    DrawDebugSphere(
        GetWorld(),
        NoiseLocation,
        80.f,
        12,
        bIsInStealth ? FColor::Blue : FColor::Green,
        false,
        1.5f
    );

    const float Loudness = bIsInStealth ? 0.25f : 1.0f;

    UAISense_Hearing::ReportNoiseEvent(
        GetWorld(),
        NoiseLocation,
        Loudness,
        this,
        0.f,
        NAME_None
    );
}

void APlayerCharacter::StartStealth()
{
    bIsInStealthUI = true;
    bIsInStealth = true;
    NoiseInterval = StealthNoiseInterval;
    GetCharacterMovement()->MaxWalkSpeed = StealthWalkSpeed;

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(2, 1.0f, FColor::Cyan, TEXT("SIGILO"));
    }
}

void APlayerCharacter::StopStealth()
{
    bIsInStealth = false;
    bIsInStealthUI = false;
    NoiseInterval = NormalNoiseInterval;
    GetCharacterMovement()->MaxWalkSpeed = NormalWalkSpeed;

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(2, 1.0f, FColor::White, TEXT("NORMAL"));
    }
}