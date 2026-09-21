// Elie Wiese-Namir © 2025. All Rights Reserved.

// ReSharper disable CppUE4CodingStandardNamingViolationWarning
// ReSharper disable CppExpressionWithoutSideEffects
// ReSharper disable CppMemberFunctionMayBeStatic
#pragma once

#include <generator>
#include <utility>

#include "flecs.h"

#include "SolidMacros/Macros.h"
#include "Types/SolidNotNull.h"
#include "Concepts/SolidConcepts.h"

#include "FlecsWorldInterfaceObject.h"
#include "Entities/FlecsComponentHandle.h"
#include "Entities/FlecsEntityRange.h"
#include "Entities/FlecsId.h"
#include "Pipelines/FlecsPipelineHandle.h"
#include "Queries/FlecsQuery.h"
#include "Worlds/FlecsWorldInterfaceObject.h"

#include "FlecsWorld.generated.h"

struct FFlecsTableHandle;
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
	
	friend class UFlecsWorldSubsystem;

public:
	UFlecsWorld(const FObjectInitializer& ObjectInitializer);
	
	virtual ~UFlecsWorld() override;

	static NO_DISCARD UFlecsWorld* GetDefaultWorld(const TSolidNotNull<const UObject*> WorldContextObject);

	void WorldStart();
 
	// ReSharper disable once CppMemberFunctionMayBeConst
	void WorldBeginPlay();

	void InitializeDefaultComponents() const;

	void InitializeFlecsRegistrationObjects();
	void CallBeginPlayForRegisteredObjects();

	void RegisterUnrealTypes() const;
	
	/**
	 * TRY NOT TO USE THIS
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
		World.each(SOLID_FWD(Function));
	}

	template <typename T, typename FunctionType>
	void ForEach(FunctionType&& Function) const
	{
		World.each<T>(SOLID_FWD(Function));
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
		std::invoke(SOLID_FWD(Function));
		EndReadOnly();
	}

	void HandleWorldPause();

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs | World")
	bool Progress(const double DeltaTime = 0.0);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs | World")
	double SetTimeScale(const double InTimeScale) const;

	void DestroyWorld();

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
	
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs | World")
	/**
	 * @brief Creates and tracks a named entity-id range for this world.
	 * @param InRangeName Non-None name used to track the range.
	 * @param InMinimum Inclusive first entity id; must be greater than zero.
	 * @param InMaximum Inclusive last entity id; zero creates an unbounded range.
	 * @return UObject wrapper for the world-owned native range.
	 *
	 * New and recycled entity ids allocated while this range is active remain
	 * within its bounds.
	 */
	UFlecsEntityRange* CreateEntityRange(const FName& InRangeName, const int32 InMinimum, const int32 InMaximum);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs | World")
	/**
	 * @brief Activates a world-owned entity range for new entity allocation.
	 * @param InEntityRange Range created by this world.
	 *
	 * New and recycled entity ids are constrained to the active range. When
	 * the range has no ids available, native Flecs creation operations assert.
	 */
	void SetActiveEntityRange(UFlecsEntityRange* InEntityRange) const;

	UFUNCTION(BlueprintCallable, Category = "Flecs | World")
	/**
	 * @brief Returns the currently active entity range.
	 * @return The active range, or nullptr when allocation is unrestricted.
	 */
	UFlecsEntityRange* GetActiveEntityRange() const;

	UFUNCTION(BlueprintCallable, Category = "Flecs | World")
	/**
	 * @brief Returns all tracked entity ranges owned by this world.
	 * @return A snapshot array of valid range wrappers.
	 */
	TArray<UFlecsEntityRange*> GetEntityRanges() const;
	
	/**
	 * @brief Lazily yields all tracked entity ranges.
	 * @return A C++ generator over world-owned range wrappers.
	 *
	 * The yielded pointers remain owned by the world and must not be retained
	 * past world reset or teardown.
	 */
	std::generator<UFlecsEntityRange*> GetEntityRangesGenerator() const;

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
	
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs | World")
	void SetPipeline(const FFlecsPipelineHandle& InPipeline) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs")
	void RunPipeline(const FFlecsId InPipeline, const double DeltaTime = 0.0) const;
	
	/*UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs")
	FFlecsQuery GetQueryFromEntity(const FFlecsEntityHandle& InEntity) const;*/
	
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
	bool UnregisterFlecsObject(const TSubclassOf<UObject>& InClass);
	
	template <Solid::TStaticClassConcept T>
	FORCEINLINE bool UnregisterFlecsObject()
	{
		return UnregisterFlecsObject(T::StaticClass());
	}
	
	UFUNCTION(BlueprintCallable, Category = "Flecs")
	UFlecsStage* GetStage(const int32 InStageId) const;
	
	NO_DISCARD UFlecsStage* GetStage(const flecs::world& InStageWorld) const;
	NO_DISCARD UFlecsStage* GetStage(const flecs::iter& InIter) const;
	
	NO_DISCARD TTuple<int32, FFlecsId> Search(const FFlecsTableHandle& InTableHandle, const FFlecsId& InId) const;
	
	void RegisterStages(const int32 InStageCount);
	
	NO_DISCARD TSolidNotNull<UFlecsStage*> CreateAsyncStage();
	
	void ImportRestModule();
	void ImportStatsModule();

	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);

	virtual void GetResourceSizeEx(FResourceSizeEx& CumulativeResourceSize) override;

	bool bIsInitialized = false;

	UPROPERTY(Transient)
	TArray<TScriptInterface<IFlecsGameLoopInterface>> GameLoopInterfaces;
	
	TMap<const UClass*, TScriptInterface<IFlecsObjectRegistrationInterface>> RegisteredObjectTypes;

	UPROPERTY(Transient)
	TArray<TScriptInterface<IFlecsObjectRegistrationInterface>> RegisteredObjects;

	TTypedFlecsQuery<FFlecsUObjectComponent> ObjectComponentQuery;

	TTypedFlecsQuery<const FFlecsScriptStructComponent> AddReferencedObjectsQuery;

	FDelegateHandle ComponentRegisteredDelegateHandle;

	FDelegateHandle ShrinkMemoryGCDelegateHandle;
	FDelegateHandle DeleteEmptyTablesGCDelegateHandle;

	UPROPERTY()
	TOptional<double> PrePauseTimeScale;
	
	// @TOOD: currently unimplemented
	UPROPERTY()
	TOptional<double> TimeScale;
	
	UPROPERTY()
	TArray<TObjectPtr<UFlecsStage>> Stages;

	UPROPERTY()
	TMap<FName, TObjectPtr<UFlecsEntityRange>> EntityRanges;

	robin_hood::unordered_flat_map<FGameplayTag, FFlecsId> TagEntityMap;
	
protected:
	virtual flecs::world* GetNativeFlecsWorld_Internal() const override
	{
		return const_cast<flecs::world*>(&World);
	}
	
	void SetContext(void* InContext) const;

private:
	flecs::world World;
	
	void CallUnregisterOnRegisteredObjects();
	
	void ExecuteInRegistrationScope(
		const TSolidNotNull<const UObject*> InObject,
		const TSolidNotNull<const IFlecsObjectRegistrationInterface*> InObjectRegistrationInterface,
		TFunctionRef<void()> InFunction);

	NO_DISCARD UFlecsEntityRange* FindTrackedEntityRange(const TSolidNotNull<const ecs_entity_range_t*> InNativeEntityRange) const;
	NO_DISCARD UFlecsEntityRange* FindTrackedEntityRange(const FName& InRangeName) const;
	UFlecsEntityRange* TrackEntityRange(const TSolidNotNull<const ecs_entity_range_t*> InNativeEntityRange, const FName& InRangeName);
	
	/**
	 * @brief Get this world as a non-const pointer
	 * @return This world as a non-const pointer
	 */
	NO_DISCARD FORCEINLINE UFlecsWorld* GetSelf() const
	{
		return const_cast<UFlecsWorld*>(this);
	}
	
}; // class UFlecsWorld
