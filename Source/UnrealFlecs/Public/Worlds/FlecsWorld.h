// Elie Wiese-Namir © 2025. All Rights Reserved.

// ReSharper disable CppUE4CodingStandardNamingViolationWarning
// ReSharper disable CppExpressionWithoutSideEffects
// ReSharper disable CppMemberFunctionMayBeStatic
#pragma once

#include <utility>

#include "flecs.h"

#include "CoreMinimal.h"

#include "SolidMacros/Macros.h"
#include "Types/SolidNotNull.h"
#include "Concepts/SolidConcepts.h"

#include "FlecsScopedDeferWindow.h"
#include "FlecsWorldInterfaceObject.h"
#include "Entities/FlecsComponentHandle.h"
#include "Entities/FlecsId.h"
#include "Queries/FlecsQuery.h"
#include "Worlds/FlecsWorldInterfaceObject.h"

#include "FlecsWorld.generated.h"

struct FFlecsTickFunction;
struct FFlecsEntityRecord;
struct FFlecsUObjectComponent;

class IFlecsObjectRegistrationInterface;
class IFlecsModuleInterface;
class IFlecsGameLoopInterface;
class UFlecsWorldSubsystem;
class UFlecsModuleInterface;
class UFlecsStage;

UCLASS(BlueprintType, NotBlueprintable)
class UNREALFLECS_API UFlecsWorld final : public UFlecsWorldInterfaceObject
{
	GENERATED_BODY()

public:
	UFlecsWorld(const FObjectInitializer& ObjectInitializer);
	
	virtual ~UFlecsWorld() override;

	static NO_DISCARD UFlecsWorld* GetDefaultWorld(const UObject* WorldContextObject);

	void WorldStart();

	// ReSharper disable once CppMemberFunctionMayBeConst
	void WorldBeginPlay();

	void InitializeDefaultComponents() const;

	void InitializeFlecsRegistrationObjects();
	void CallBeginPlayForRegisteredObjects();

	void RegisterUnrealTypes() const;

	/**
	 * @brief HACKY: Progress the iterator while unlocking the table lock for the duration of the function call.
	 * does not loop over the iterator, only progresses it after each function call.
	 * @param Iter The Flecs iterator.
	 * @param Function The function to execute.
	 */
	void UnlockIter_Internal(flecs::iter& Iter, TFunctionRef<void(flecs::iter&)> Function) const
	{
		DeferEndLambda([this, &Iter, Function]()
		{
			if (IsReadOnly())
			{
				EndReadOnly();
			}

			while (Iter.next())
			{
				const int32 SavedLockCount = internal_ecs_table_disable_lock(Iter.table());

				std::invoke(Function, Iter);

				internal_ecs_table_enable_lock(Iter.table(), SavedLockCount);
			}

			if (IsReadOnly())
			{
				BeginReadOnly();
			}
		});
	}

	void InitializeComponentPropertyObserver();
	void InitializeSystems();

	/**
	 * @brief Deletes and recreates the world, 
	 */
	UFUNCTION()
	void Reset();

	/**
	 * @brief Reset simulation clock.
	 * @see ecs_reset_clock
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs | World")
	void ResetClock() const;
	
	template <typename T>
	FORCEINLINE FFlecsEntityHandle ImportFlecsModule()
	{
		return World.import<T>();
	}

	template <typename FunctionType>
	void ForEach(FunctionType&& Function) const
	{
		World.each(std::forward<FunctionType>(Function));
	}

	template <typename T, typename FunctionType>
	void ForEach(FunctionType&& Function) const
	{
		World.each<T>(std::forward<FunctionType>(Function));
	}

	template <typename FunctionType>
	void ForEach(const FFlecsId& InTermId, const FunctionType& Function) const
	{
		World.each(InTermId, Function);
	}

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs | World")
	bool BeginReadOnly() const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs | World")
	void EndReadOnly() const;

	template <typename TFunction>
	void ReadOnly(TFunction&& Function) const
	{
		BeginReadOnly();
		std::invoke(std::forward<TFunction>(Function));
		EndReadOnly();
	}

	void SetContext(void* InContext) const;

	void HandleWorldPause();

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs | World")
	bool ProgressGameLoops(const FGameplayTag& TickTypeTag, const double DeltaTime = 0.0);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs | World")
	bool Progress(const double DeltaTime = 0.0);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs | World")
	double SetTimeScale(const double InTimeScale) const;

	void DestroyWorld();

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs | World")
	void SetPipeline(const FFlecsEntityHandle& InPipeline) const;

	template <typename T>
	void SetPipeline() const
	{
		World.set_pipeline<T>();
	}
	
	UFUNCTION(BlueprintCallable, Category = "Flecs | World")
	void SetStageCount(const int32 InStageCount);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs | World")
	void PreallocateEntities(const int32 InEntityCount) const;
	
	UFUNCTION(BlueprintCallable, Category = "Flecs | World")
	void SetThreads(const int32 InThreadCount);

	UFUNCTION(BlueprintCallable, Category = "Flecs | World")
	void SetTaskThreads(const int32 InThreadCount);

	// @TODO: Re-implement bitmask registration
	/*
	 FFlecsEntityHandle RegisterComponentBitmaskType(const UEnum* ScriptEnum) const
	{
		solid_check(IsValid(ScriptEnum));

		const FFlecsEntityHandle OldScope = ClearScope();

		solid_checkf(!TypeMapComponent->ScriptEnumMap.contains(ScriptEnum),
			TEXT("Script enum %s is already registered"), *ScriptEnum->GetName());

		flecs::untyped_component ScriptEnumComponent;

		DeferEndScoped([this, ScriptEnum, &ScriptEnumComponent]()
		{
			ScriptEnumComponent = World.component(StringCast<char>(*ScriptEnum->GetName()).Get());
			solid_check(ScriptEnumComponent.is_valid());
			ScriptEnumComponent.set_symbol(StringCast<char>(*ScriptEnum->GetName()).Get());
			ScriptEnumComponent.set<flecs::Component>(
				{ .size = sizeof(uint8), .alignment = alignof(uint8) });
			ScriptEnumComponent.add<flecs::Bitmask>();

			const int32 EnumCount = ScriptEnum->NumEnums();
			
			for (int32 EnumIndex = 0; EnumIndex < EnumCount; ++EnumIndex)
			{
				const FString EnumName = ScriptEnum->GetNameStringByIndex(EnumIndex);
				const int32 EnumValue = ScriptEnum->GetValueByIndex(EnumIndex);
				
				ScriptEnumComponent.bit<uint8>(StringCast<char>(*EnumName).Get(), EnumValue, flecs::U8);
			}

			if (!flecs::_::g_type_to_impl_data.contains(
				std::string(StringCast<char>(*ScriptEnum->GetName()).Get())))
			{
				flecs::_::type_impl_data NewData;
				NewData.s_index = flecs_component_ids_index_get();
				NewData.s_size = sizeof(uint8);
				NewData.s_alignment = alignof(uint8);
				NewData.s_allow_tag = true;
				
				flecs::_::g_type_to_impl_data.emplace(
					std::string(StringCast<char>(*ScriptEnum->GetName()).Get()), NewData);
			}

			solid_check(flecs::_::g_type_to_impl_data.contains(
				std::string(StringCast<char>(*ScriptEnum->GetName()).Get())));
			flecs::_::type_impl_data& Data = flecs::_::g_type_to_impl_data.at(
				std::string(StringCast<char>(*ScriptEnum->GetName()).Get()));

			flecs_component_ids_set(World, Data.s_index, ScriptEnumComponent);
			TypeMapComponent->ScriptEnumMap.emplace(ScriptEnum, ScriptEnumComponent);
		});

		ScriptEnumComponent.set<FFlecsScriptEnumComponent>({ ScriptEnum });
		SetScope(OldScope);
		return ScriptEnumComponent;
	}
	*/

	template <typename T>
	FFlecsComponentHandle ObtainComponentTypeStruct() const
	{
		solid_checkf(World.is_valid(flecs::_::type<T>::id(World)),
			TEXT("Component %hs is not registered"), nameof(T).data());
		return World.component<T>();
	}

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs")
	void RunPipeline(const FFlecsId InPipeline, const double DeltaTime = 0.0) const;
	
	/*UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs")
	FFlecsQuery GetQueryFromEntity(const FFlecsEntityHandle& InEntity) const;*/

	virtual bool IsSupportedForNetworking() const override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs")
	void ShrinkWorld() const;

	/**
	 * @brief Cleanup empty tables.
	 * This operation cleans up empty tables that meet certain conditions. Having
	 * large amounts of empty tables does not negatively impact performance of the
	 * ECS, but can take up considerable amounts of memory, especially in
	 * applications with many components, and many components per entity.
	 *
	 * The generation specifies the minimum number of times this operation has
	 * to be called before an empty table is cleaned up. If a table becomes non
	 * empty, the generation is reset.
	 *
	 * The operation allows for both a "clear" generation and a "delete"
	 * generation. When the clear generation is reached, the table's
	 * resources are freed (like component arrays) but the table itself is not
	 * deleted. When the delete generation is reached, the empty table is deleted.
	 *
	 * By specifying a non-zero id the cleanup logic can be limited to tables with
	 * a specific (component) id. The operation will only increase the generation
	 * count of matching tables.
	 *
	 * The min_id_count specifies a lower bound for the number of components a table
	 * should have. Often the more components a table has, the more specific it is
	 * and therefore less likely to be reused.
	 *
	 * The time budget specifies how long the operation should take at most.
	 *
	 * @param TimeBudgetSeconds The time budget in seconds.
	 * @param ClearGeneration The generation after which to clear empty tables.
	 * @param DeleteGeneration The generation after which to delete empty tables.
	 */
	UFUNCTION()
	int32 DeleteEmptyTables(const double TimeBudgetSeconds, const uint16 ClearGeneration = 1,
	                        const uint16 DeleteGeneration = 1) const;

	NO_DISCARD FFlecsEntityHandle GetFlecsTickFunctionByType(const FGameplayTag& InTickType) const;
	
	// CAN RETURN NULL
	UFUNCTION(BlueprintCallable, Category = "Flecs")
	UObject* RegisterFlecsObject(const TSubclassOf<UObject> InClass);
	
	// CAN RETURN NULL
	template <Solid::TStaticClassConcept T>
	FORCEINLINE T* RegisterFlecsObject()
	{
		return Cast<T>(RegisterFlecsObject(T::StaticClass()));
	}
	
	UFUNCTION(BlueprintCallable, Category = "Flecs")
	UFlecsStage* GetStage(const int32 InStageId) const;
	
	void RegisterStages(const int32 InStageCount);
	
	NO_DISCARD UFlecsStage* CreateAsyncStage();
	
	void ImportRestModule();
	void ImportStatsModule();

	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);

	virtual void GetResourceSizeEx(FResourceSizeEx& CumulativeResourceSize) override;

	bool bIsInitialized = false;

	UPROPERTY(Transient)
	TArray<TScriptInterface<IFlecsGameLoopInterface>> GameLoopInterfaces;

	TSortedMap<FGameplayTag, TArray<TScriptInterface<IFlecsGameLoopInterface>>> GameLoopTickTypes;
	
	TMap<const UClass*, TScriptInterface<IFlecsObjectRegistrationInterface>> RegisteredObjectTypes;

	UPROPERTY(Transient)
	TArray<TScriptInterface<IFlecsObjectRegistrationInterface>> RegisteredObjects;

	TTypedFlecsQuery<FFlecsUObjectComponent> ObjectComponentQuery;

	FFlecsQuery TickFunctionQuery;

	TTypedFlecsQuery<const FFlecsScriptStructComponent> AddReferencedObjectsQuery;

	FDelegateHandle ComponentRegisteredDelegateHandle;

	FDelegateHandle ShrinkMemoryGCDelegateHandle;
	FDelegateHandle DeleteEmptyTablesGCDelegateHandle;

	UPROPERTY()
	TOptional<double> PrePauseTimeScale;
	
	UPROPERTY()
	TArray<TObjectPtr<UFlecsStage>> Stages;

	robin_hood::unordered_flat_map<FGameplayTag, FFlecsId> TagEntityMap;
	
protected:
	virtual flecs::world* GetNativeFlecsWorld_Internal() const override
	{
		return const_cast<flecs::world*>(&World);
	}

private:
	flecs::world World;
	
	void CallUnregisterOnRegisteredObjects();
	
	/**
	 * @brief Get this world as a non-const pointer
	 * @return This world as a non-const pointer
	 */
	NO_DISCARD FORCEINLINE UFlecsWorld* GetSelf() const
	{
		return const_cast<UFlecsWorld*>(this);
	}
	
}; // class UFlecsWorld
