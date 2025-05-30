﻿// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TickerPhysicsHistoryComponent.h"
#include "Modules/FlecsModuleObject.h"
#include "Systems/FlecsSystem.h"
#include "Ticker/FlecsTickerComponent.h"
#include "UObject/Object.h"
#include "FlecsPhysicsModule.generated.h"

// @TODO: Not Fully Implemented
UCLASS(BlueprintType, DefaultToInstanced, EditInlineNew, DisplayName = "Flecs Physics Module")
class UNREALFLECS_API UFlecsPhysicsModule final : public UFlecsModuleObject
{
	GENERATED_BODY()

public:
	virtual void InitializeModule(TSolidNonNullPtr<UFlecsWorld> InWorld, const FFlecsEntityHandle& InModuleEntity) override;
	virtual void DeinitializeModule(TSolidNonNullPtr<UFlecsWorld> InWorld) override;

	virtual void WorldBeginPlay(TSolidNonNullPtr<UFlecsWorld> InWorld, TSolidNonNullPtr<UWorld> InGameWorld) override;

	void ResimulationHandlers();

	FORCEINLINE virtual FString GetModuleName_Implementation() const override
	{
		return "Flecs Physics Module";
	}

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	bool bAllowResimulation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics", meta = (EditCondition = "bAllowResimulation"))
	int32 MaxFrameHistory = 300;

	flecs::observer AddPhysicsComponentObserver;

private:
	int32 PreResimValue = 0;

	FTickerPhysicsHistoryComponent* PhysicsHistoryComponentRef;
	FFlecsTickerComponent* TickerComponentRef;

	FPhysScene* Scene = nullptr;
	
}; // class UFlecsPhysicsModule
