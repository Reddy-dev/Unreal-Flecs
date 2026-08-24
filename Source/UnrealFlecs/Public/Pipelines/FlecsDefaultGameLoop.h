// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "FlecsPipelineHandle.h"
#include "FlecsGameLoopObject.h"

#include "FlecsDefaultGameLoop.generated.h"

UCLASS(BlueprintType)
class UNREALFLECS_API UFlecsDefaultGameLoop : public UFlecsGameLoopObject
{
	GENERATED_BODY()

public:
	UFlecsDefaultGameLoop();
	
	virtual void InitializeGameLoop(TSolidNotNull<UFlecsWorld*> InWorld, const FFlecsEntityHandle& InGameLoopEntity) override;
	virtual bool Progress(double DeltaTime, const FGameplayTag& InTickType, TSolidNotNull<UFlecsWorld*> InWorld) override;
	
	virtual bool IsMainLoop() const override;
	virtual TArray<FGameplayTag> GetTickTypeTags() const override;
	
	UPROPERTY(EditAnywhere)
	bool bUsePhasesInUnrealTickGroups = false;

	// Main Loop
	UPROPERTY()
	FFlecsPipelineHandle MainLoopPipeline;
	
	UPROPERTY()
	FFlecsPipelineHandle PrePhysicsPipeline;

	UPROPERTY()
	FFlecsPipelineHandle DuringPhysicsPipeline;

	UPROPERTY()
	FFlecsPipelineHandle PostPhysicsPipeline;

	UPROPERTY()
	FFlecsPipelineHandle PostUpdateWorkPipeline;

protected:
	NO_DISCARD FFlecsPipelineHandle CreatePipelineForTickType(const FGameplayTag& InTickType, TSolidNotNull<UFlecsWorld*> InWorld) const;
	
}; // class UFlecsDefaultGameLoop
