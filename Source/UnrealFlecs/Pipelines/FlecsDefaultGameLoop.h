// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FlecsGameLoopObject.h"
#include "FlecsGameLoopPhaseTree.h"
#include "Entities/FlecsEntityHandle.h"
#include "FlecsDefaultGameLoop.generated.h"

UCLASS(BlueprintType)
class UNREALFLECS_API UFlecsDefaultGameLoop final : public UFlecsGameLoopObject
{
	GENERATED_BODY()

public:
	UFlecsDefaultGameLoop();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config")
	FFlecsGameLoopPhaseTree PhaseTree;

	UPROPERTY()
	FFlecsEntityHandle GameLoopEntity;
	
	virtual void InitializeGameLoop(TSolidNotNull<UFlecsWorld*> InWorld) override;
	virtual bool Progress(double DeltaTime, TSolidNotNull<UFlecsWorld*> InWorld) override;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif // WITH_EDITOR
	
}; // class UFlecsDefaultGameLoop