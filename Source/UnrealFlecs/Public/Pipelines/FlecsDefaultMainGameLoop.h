// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "FlecsPipelineHandle.h"
#include "FlecsGameLoopObject.h"

#include "FlecsDefaultMainGameLoop.generated.h"

UCLASS(BlueprintType)
class UNREALFLECS_API UFlecsDefaultMainGameLoop : public UFlecsGameLoopObject
{
	GENERATED_BODY()

public:
	UFlecsDefaultMainGameLoop();
	
	virtual void InitializeGameLoop(TSolidNotNull<UFlecsWorld*> InWorld, const FFlecsEntityHandle& InGameLoopEntity) override;
	virtual void DeinitializeGameLoop(TSolidNotNull<UFlecsWorld*> InWorld, const FFlecsEntityHandle& InGameLoopEntity) override;
	
	virtual bool Progress(double DeltaTime, TSolidNotNull<UFlecsWorld*> InWorld, ELevelTick InTickType, ENamedThreads::Type InCurrentThread,
		const FGraphEventRef& InCompletionGraphEvent) override;
	
	virtual bool IsMainLoop() const override;

	// Main Loop
	UPROPERTY()
	FFlecsPipelineHandle MainLoopPipeline;
	
	/*UPROPERTY()
	FFlecsPipelineHandle PrePhysicsPipeline;

	UPROPERTY()
	FFlecsPipelineHandle DuringPhysicsPipeline;

	UPROPERTY()
	FFlecsPipelineHandle PostPhysicsPipeline;

	UPROPERTY()
	FFlecsPipelineHandle PostUpdateWorkPipeline;*/
	
}; // class UFlecsDefaultGameLoop
