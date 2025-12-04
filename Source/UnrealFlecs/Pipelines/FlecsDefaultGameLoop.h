// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FlecsGameLoopObject.h"
#include "FlecsDefaultGameLoop.generated.h"

UCLASS(BlueprintType)
class UNREALFLECS_API UFlecsDefaultGameLoop final : public UFlecsGameLoopObject
{
	GENERATED_BODY()

public:
	virtual void InitializeGameLoop(TSolidNotNull<UFlecsWorld*> InWorld, const FFlecsEntityHandle& InGameLoopEntity) override;
	virtual bool Progress(double DeltaTime, const FGameplayTag& InTickType, TSolidNotNull<UFlecsWorld*> InWorld) override;
	
	virtual bool IsMainLoop() const override;
	virtual TArray<FGameplayTag> GetTickTypeTags() const override;

	// Main Loop
	UPROPERTY()
	FFlecsEntityHandle MainLoopPipeline;
	
	UPROPERTY()
	FFlecsEntityHandle PrePhysicsPipeline;

	UPROPERTY()
	FFlecsEntityHandle DuringPhysicsPipeline;

	UPROPERTY()
	FFlecsEntityHandle PostPhysicsPipeline;

	UPROPERTY()
	FFlecsEntityHandle PostUpdateWorkPipeline;
	
}; // class UFlecsDefaultGameLoop
