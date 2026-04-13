#include "EnvQueryContext_Player.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

void UEnvQueryContext_Player::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
    UObject* QueryOwner = QueryInstance.Owner.Get();
    if (!QueryOwner)
    {
        return;
    }

    UWorld* World = QueryOwner->GetWorld();
    if (!World)
    {
        return;
    }

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn)
    {
        return;
    }

    UEnvQueryItemType_Actor::SetContextHelper(ContextData, PlayerPawn);
}