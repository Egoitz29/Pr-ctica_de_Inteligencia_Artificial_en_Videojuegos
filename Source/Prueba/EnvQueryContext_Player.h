#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "EnvQueryContext_Player.generated.h"

UCLASS()
class PRUEBA_API UEnvQueryContext_Player : public UEnvQueryContext
{
    GENERATED_BODY()

public:
    virtual void ProvideContext(struct FEnvQueryInstance& QueryInstance, struct FEnvQueryContextData& ContextData) const override;
};