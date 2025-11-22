// Elie Wiese-Namir © 2025. All Rights Reserved.

// ReSharper disable CppMemberFunctionMayBeStatic
// ReSharper disable CppExpressionWithoutSideEffects
// ReSharper disable CppUnusedIncludeDirective
#pragma once

#include <thread>
#include <unordered_map>

#include "flecs.h"

#include "CoreMinimal.h"

#include "Subsystems/WorldSubsystem.h"

#include "Types/SolidNotNull.h"

#include "Pipelines/FlecsTickingGroup.h"
#include "Tick/FlecsWorldTickFunction.h"

#include "FlecsWorldSubsystem.generated.h"

struct FFlecsWorldSettingsInfo;

class UFlecsWorldSubsystem;
class UFlecsWorld;

DECLARE_MULTICAST_DELEGATE_OneParam(FFlecsOnWorldCreated, TSolidNotNull<UFlecsWorld*>);
DECLARE_MULTICAST_DELEGATE_OneParam(FFlecsOnWorldBeginPlay, TSolidNotNull<UWorld*>);
DECLARE_MULTICAST_DELEGATE_OneParam(FFlecsOnWorldDestroyed, TSolidNotNull<UFlecsWorld*>);
DECLARE_MULTICAST_DELEGATE_OneParam(FFlecsOnWorldInitializedGlobal, TSolidNotNull<UFlecsWorld*>);

namespace Unreal::Flecs
{
	// A Global alternative to be able to register to the delegate outside of a world context.
	extern UNREALFLECS_API FFlecsOnWorldInitializedGlobal GOnFlecsWorldInitialized;
	
} // namespace Unreal::Flecs

UCLASS(BlueprintType)
class UNREALFLECS_API UFlecsWorldSubsystem final : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UFlecsWorldSubsystem();
	
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	virtual void Deinitialize() override;

	UFUNCTION()
	UFlecsWorld* CreateWorld(const FString& Name, const FFlecsWorldSettingsInfo& Settings);

	void SetWorld(UFlecsWorld* InWorld);

	UFUNCTION(BlueprintCallable, Category = "Flecs")
	UFlecsWorld* GetDefaultWorld() const;

	NO_DISCARD TSolidNotNull<UFlecsWorld*> GetDefaultWorldChecked() const;

	UFUNCTION(BlueprintCallable, Category = "Flecs")
	bool HasValidFlecsWorld() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs", Meta = (WorldContext = "WorldContextObject"))
	static UFlecsWorld* GetDefaultWorldStatic(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Flecs", Meta = (WorldContext = "WorldContextObject"))
	static bool HasValidFlecsWorldStatic(const UObject* WorldContextObject);

	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	void ListenBeginPlay(const FFlecsOnWorldBeginPlay::FDelegate& Delegate);

	FFlecsOnWorldCreated OnWorldCreatedDelegate;
	FFlecsOnWorldBeginPlay OnWorldBeginPlayDelegate;
	FFlecsOnWorldDestroyed OnWorldDestroyedDelegate;

	FFlecsWorldTickFunction PrePhysicsTickFunction;
	FFlecsWorldTickFunction DuringPhysicsTickFunction;
	FFlecsWorldTickFunction PostPhysicsTickFunction;
	FFlecsWorldTickFunction PostUpdateWorkTickFunction;

	void SetupTickFunctions(const TSolidNotNull<UFlecsWorld*> InFlecsWorld);

	void SetupTickFunctionSettings(const TSolidNotNull<UFlecsWorld*> InFlecsWorld, const FFlecsWorldTickFunction& InTickFunction);

protected:
	UPROPERTY(Transient)
	TObjectPtr<UFlecsWorld> DefaultWorld;

	void RegisterAllGameplayTags(const TSolidNotNull<UFlecsWorld*> InFlecsWorld);

}; // class UFlecsWorldSubsystem
