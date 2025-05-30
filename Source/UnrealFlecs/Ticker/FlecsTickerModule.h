﻿// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FlecsTickerComponent.h"
#include "Modules/FlecsModuleObject.h"
#include "Modules/FlecsModuleProgressInterface.h"
#include "Systems/FlecsSystem.h"
#include "UObject/Object.h"
#include "FlecsTickerModule.generated.h"

UCLASS(BlueprintType, DisplayName = "Flecs Ticker Module")
class UNREALFLECS_API UFlecsTickerModule final : public UFlecsModuleObject, public IFlecsModuleProgressInterface
{
	GENERATED_BODY()

public:
	UFlecsTickerModule(const FObjectInitializer& InObjectInitializer);

	virtual void InitializeModule(TSolidNonNullPtr<UFlecsWorld> InWorld, const FFlecsEntityHandle& InModuleEntity) override;
	virtual void DeinitializeModule(TSolidNonNullPtr<UFlecsWorld> InWorld) override;

	virtual void ProgressModule(double InDeltaTime) override;

	FORCEINLINE virtual FString GetModuleName_Implementation() const override
	{
		return "Flecs Ticker Module";
	}

	UPROPERTY()
	bool bUsePhysicsTick = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ticker",
		meta = (Units = "Hz", ClampMin = "1", ClampMax = "240", EditCondition = "!bUsePhysicsTick"))
	int64 TickerRate = 60;

	UFUNCTION(BlueprintCallable, Category = "Flecs | Ticker")
	FORCEINLINE int64 GetTickerRate() const { return TickerRate; }

	UFUNCTION(BlueprintCallable, Category = "Flecs | Ticker")
	FORCEINLINE void SetTickerRate(const int64 InRate) { TickerRate = InRate; }

	UFUNCTION(BlueprintCallable, Category = "Flecs | Ticker")
	FORCEINLINE FFlecsSystem GetTickerSystem() const { return TickerSystem; }

	UFUNCTION(BlueprintCallable, Category = "Flecs | Ticker")
	FORCEINLINE FFlecsEntityHandle GetTickerSource() const { return TickerSystem.GetEntity(); }

	UPROPERTY()
	double TickerAccumulator = 0.0;

	double TickerInterval = 0.0;

	FFlecsTickerComponent* TickerComponentPtr = nullptr;

	UPROPERTY()
	FFlecsSystem TickerSystem;

	UPROPERTY()
	FFlecsEntityHandle MainPipeline;

	UPROPERTY()
	FFlecsEntityHandle TickerPipeline;

}; // class UFlecsTickerModule
