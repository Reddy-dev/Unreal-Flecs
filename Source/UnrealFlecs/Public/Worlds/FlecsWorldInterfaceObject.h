// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "flecs.h"

#include "CoreMinimal.h"

#include "UObject/Object.h"

#include "SolidMacros/Macros.h"

#include "FlecsScopedDeferWindow.h"
#include "Entities/FlecsEntityHandle.h"
#include "Entities/FlecsComponentHandle.h"
#include "Entities/FlecsId.h"
#include "Observers/FlecsObserverBuilder.h"
#include "Systems/FlecsSystemBuilder.h"
#include "Timers/FlecsTimerHandle.h"

#include "FlecsWorldInterfaceObject.generated.h"

class UFlecsStage;
struct FFlecsEntityRecord;

class UFlecsWorld;

/**
 * 
 */
UCLASS(Abstract, BlueprintType)
class UNREALFLECS_API UFlecsWorldInterfaceObject : public UObject
{
	GENERATED_BODY()

public:
	UFlecsWorldInterfaceObject(const FObjectInitializer& ObjectInitializer);
	
	virtual ~UFlecsWorldInterfaceObject() override;
	
	UFUNCTION(BlueprintCallable, Category = "Flecs|World")
	UFlecsWorld* GetFlecsWorld() const;
	
	UFUNCTION(BlueprintCallable, Category = "Flecs | World")
	UFlecsStage* AsStage() const;
	
	NO_DISCARD flecs::world GetNativeFlecsWorld() const;
	
	UFUNCTION(BlueprintCallable, Category = "Flecs | World")
	double GetTimeScale() const;
	
	UFUNCTION(BlueprintCallable, Category = "Flecs|World")
	bool IsStage() const;
	
	/**
	 * Test whether the current world object is readonly.
	 * This function allows the code to test whether the currently used world
	 * object is readonly or whether it allows for writing.
	 *
	 * @return True if the world or stage is readonly.
	 *
	 * @see ecs_stage_is_readonly()
	 * @see flecs::world::readonly_begin()
	 * @see flecs::world::readonly_end()
	 */
	UFUNCTION(BlueprintCallable, Category = "Flecs | World")
	bool IsReadOnly() const;
	
	UFUNCTION(BlueprintCallable, Category = "Flecs | World")
	bool ShouldQuit() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs | World")
	bool BeginDefer() const;

	NO_DISCARD FFlecsScopedDeferWindow DeferWindow() const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs | World")
	bool EndDefer() const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs | World")
	void ResumeDefer() const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs | World")
	void SuspendDefer() const;

	UFUNCTION(BlueprintCallable, Category = "Flecs | World")
	bool IsDeferred() const;
	
	UFUNCTION(BlueprintCallable, Category = "Flecs | World")
	bool IsDeferSuspended() const;

	template <typename TFunction>
	void Defer(TFunction&& Function) const
	{
		GetNativeFlecsWorld_Internal()->defer<TFunction>(std::forward<TFunction>(Function));
	}

	/**
	 * @brief End or Suspend the defer state and execute the lambda function.
	 * @param Function The function to execute after the defer state is ended or suspended.
	 * @param bEndDefer If true, the defer state will be ended after the function is executed, otherwise it will be suspended and resumed after the function is executed.
	 */
	template <typename TFunction>
	void DeferEndLambda(TFunction&& Function, const bool bEndDefer = false) const
	{
		const bool bIsDeferred = IsDeferred();
		
		if (bIsDeferred)
		{
			if (bEndDefer)
			{
				EndDefer();
			}
			else
			{
				SuspendDefer();
			}
		}

		std::invoke(std::forward<TFunction>(Function));

		if (bIsDeferred)
		{
			if (bEndDefer)
			{
				BeginDefer();
			}
			else
			{
				ResumeDefer();
			}
		}
	}
	
	UFUNCTION(BlueprintCallable, Category = "Flecs | World")
	bool DoesExist(const FFlecsId InId) const;
	
	UFUNCTION(BlueprintCallable, Category = "Flecs | World")
	bool IsAlive(const FFlecsId InId) const;
	
	UFUNCTION(BlueprintCallable, Category = "Flecs | World")
	bool IsValidId(const FFlecsId InId) const;
	
	UFUNCTION(BlueprintCallable, Category = "Flecs | World")
	FFlecsEntityHandle GetAlive(const FFlecsId InId) const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs | World")
	FFlecsEntityHandle MakeAlive(const FFlecsId InId) const;
	
	UFUNCTION(BlueprintCallable, Category = "Flecs | World")
	double GetDeltaTime() const;
	
	template <typename T>
	NO_DISCARD FFlecsId GetIdIfRegistered() const
	{
		return GetNativeFlecsWorld_Internal()->id_if_registered<T>();
	}
	
	NO_DISCARD void* GetContext() const;
	
	NO_DISCARD FFlecsTypeMapComponent* GetTypeMapComponent() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs | World")
	FFlecsId SetScope(const FFlecsId InScope) const;
	
	UFUNCTION(BlueprintCallable, Category = "Flecs | World")
	FFlecsId GetScope() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs | World")
	FFlecsId ClearScope() const;
	
	template <typename T>
	NO_DISCARD FFlecsId SetScope() const
	{
		return GetNativeFlecsWorld_Internal()->set_scope<T>();
	}
	
	template <typename T>
	void Scope() const
	{
		GetNativeFlecsWorld_Internal()->scope<T>();
	}

	template <typename T, typename FunctionType>
	void Scope(FunctionType&& Function) const
	{
		GetNativeFlecsWorld_Internal()->scope<T>(std::forward<FunctionType>(Function));
	}

	template <typename FunctionType>
	void Scope(FFlecsId InId, FunctionType&& Function) const
	{
		GetNativeFlecsWorld_Internal()->scope(InId, std::forward<FunctionType>(Function));
	}

	template <typename FunctionType>
	void EndScope(FunctionType&& Function) const
	{
		const flecs::entity OldScope = GetNativeFlecsWorld_Internal()->set_scope(0);

		std::invoke(std::forward<FunctionType>(Function));

		GetNativeFlecsWorld_Internal()->set_scope(OldScope);
	}
	
	/**
	 * @brief Lookup an entity by name, if it does not exist, it will return an invalid handle
	 * @param Name The name of the entity to lookup
	 * @param Separator The separator to use for nested scopes, defaults to "::"
	 * @param RootSeparator The root separator, if the name starts with this, it will search from the root scope
	 * @param bRecursive If true, will search recursively in the scopes(Up), otherwise only in the current scope
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs | World",
		meta = (AdvancedDisplay = "Separator, RootSeparator, bRecursive"))
	FFlecsEntityHandle LookupEntity(const FString& Name,
									const FString& Separator = "::",
									const FString& RootSeparator = "::",
									const bool bRecursive = true) const;
	
	NO_DISCARD FFlecsEntityHandle LookupEntityBySymbol_Internal(const FString& Symbol,
		const bool bLookupAsPath = false, const bool bRecursive = true) const;
	
	/**
	 * @brief Add a singleton component to the world
	 * @tparam T The component type
	 * @return The world object for chaining/builder pattern
	 */
	template <typename T>
	UFlecsWorldInterfaceObject* Add() const
	{
		GetNativeFlecsWorld_Internal()->add<T>();
		return GetSelfInterface_Internal();
	}
	
	/**
	 * @brief Set the value of a singleton component in the world, if the singleton does not exist, it will be created
	 * @tparam T The component type
	 * @param Value The value to set the singleton to
	 * @return The world object for chaining/builder pattern
	 */
	template <typename T>
	UFlecsWorldInterfaceObject* Set(const T& Value) const
	{
		GetNativeFlecsWorld_Internal()->set<T>(Value);
		return GetSelfInterface_Internal();
	}

	template <typename T>
	UFlecsWorldInterfaceObject* Set(T&& Value) const
	{
		GetNativeFlecsWorld_Internal()->set<T>(FLECS_FWD(Value));
		return GetSelfInterface_Internal();
	}
	
	/**
	 * @brief Remove a singleton component from the world
	 * @tparam T The component type
	 * @return The world object for chaining/builder pattern
	 */
	template <typename T>
	UFlecsWorldInterfaceObject* Remove() const
	{
		GetNativeFlecsWorld_Internal()->remove<T>();
		return GetSelfInterface_Internal();
	}
	
	/**
	 * @brief Check if the world has a singleton component of the given type
	 * @tparam T The component type, this must be already registered in the world or it will assert
	 * @return True if the singleton exists, false otherwise
	 */
	template <typename T>
	NO_DISCARD bool Has() const
	{
		return GetNativeFlecsWorld_Internal()->has<T>();
	}

	/**
	 * @brief Get a singleton component from the world, will assert if the singleton does not exist
	 * @tparam T The component type
	 * @return The singleton component
	 */
	template <typename T>
	NO_DISCARD const T& Get() const
	{
		solid_checkf(Has<T>(), TEXT("Singleton %hs not found"), nameof(T).data());
		return GetNativeFlecsWorld_Internal()->get<T>();
	}

	/**
	 * @brief Get a mutable singleton component from the world, will assert if the singleton does not exist
	 * @tparam T The component type
	 * @return The singleton component
	 */
	template <typename T>
	NO_DISCARD T& GetMut() const
	{
		solid_checkf(Has<T>(), TEXT("Singleton %hs not found"), nameof(T).data());
		return GetNativeFlecsWorld_Internal()->get_mut<T>();
	}
	
	/**
	 * @brief Try to get a singleton component from the world, will return nullptr if the singleton does not exist
	 * @tparam T The component type
	 * @return The singleton component or nullptr if it does not exist
	 */
	template <typename T>
	NO_DISCARD const T* TryGet() const
	{
		return GetNativeFlecsWorld_Internal()->try_get<T>();
	}

	/**
	 * @brief Try to get a mutable singleton component from the world, will return nullptr if the singleton does not exist
	 * @tparam T The component type
	 * @return The singleton component or nullptr if it does not exist
	 */
	template <typename T>
	NO_DISCARD T* TryGetMut() const
	{
		return GetNativeFlecsWorld_Internal()->try_get_mut<T>();
	}
	
	/**
	 * @brief Notify the world that a singleton component has been modified
	 * @tparam T The singleton type, must be registered in the world
	 */
	template <typename T>
	void Modified() const
	{
		GetNativeFlecsWorld_Internal()->modified<T>();
	}
	
	/**
	 * @brief Obtain the entity handle of a singleton component, will assert if the singleton does not exist
	 * @tparam T The singleton type, must be registered in the world
	 * @return The entity handle of the singleton
	 */
	template <typename T>
	FFlecsEntityHandle ObtainSingletonEntity() const
	{
		solid_checkf(Has<T>(), TEXT("Singleton %hs not found"), nameof(T).data());
		return GetNativeFlecsWorld_Internal()->singleton<T>();
	}
	
	UFUNCTION(BlueprintCallable, Category = "Flecs | World")
	int32 Count(const FFlecsId InComponentId) const;
	
	template <typename T>
	NO_DISCARD int32 Count() const
	{
		return GetNativeFlecsWorld_Internal()->count<T>();
	}
	
	UFUNCTION(BlueprintCallable, Category = "Flecs | World")
	int32 CountPair(const FFlecsId InFirstId, const FFlecsId InSecondId) const;
	
	template <typename TFirst>
	NO_DISCARD int32 CountPair(const FFlecsId InSecondId) const
	{
		return GetNativeFlecsWorld_Internal()->count<TFirst>(InSecondId);
	}
	
	template <typename TFirst, typename TSecond>
	NO_DISCARD int32 CountPair() const
	{
		return GetNativeFlecsWorld_Internal()->count<TFirst, TSecond>();
	}
	
	UFUNCTION(BlueprintCallable, Category = "Flecs")
	bool IsIdInUse(const FFlecsId InId) const;

	UFUNCTION(BlueprintCallable, Category = "Flecs")
	FFlecsId GetTypeId(const FFlecsId InId) const;

	UFUNCTION(BlueprintCallable, Category = "Flecs")
	bool IsIdType(const FFlecsId InId) const;

	UFUNCTION(BlueprintCallable, Category = "Flecs")
	bool IsIdTag(const FFlecsId InId) const;
	
	void RegisterMemberProperties(const TSolidNotNull<const UStruct*> InStruct, const FFlecsComponentHandle& InComponent) const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs")
	FFlecsEntityHandle RegisterScriptStruct(const UScriptStruct* ScriptStruct, const bool bComponent = true, const bool bRegisterMemberProperties = true) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs")
	FFlecsEntityHandle RegisterScriptEnum(const UEnum* ScriptEnum) const;

	template <typename T>
	requires (std::is_enum<T>::value)
	FFlecsEntityHandle RegisterScriptEnum() const
	{
		solid_checkf(!IsDeferred(), TEXT("Cannot register component types while deferred"));
		return GetNativeFlecsWorld_Internal()->component<T>();
	}
	
	FFlecsEntityHandle RegisterComponentEnumType(TSolidNotNull<const UEnum*> ScriptEnum) const;

	FFlecsEntityHandle RegisterScriptClassType(TSolidNotNull<UClass*> ScriptClass) const;
	
	template <typename T>
	TFlecsComponentHandle<T> RegisterComponentType() const
	{
		solid_checkf(!IsDeferred(), TEXT("Cannot register component while deferred"));
		
		TFlecsComponentHandle<T> Component = GetNativeFlecsWorld_Internal()->component<T>();
		solid_check(Component.IsValid());

		return Component;
	}
	
	template <Solid::TScriptStructConcept T>
	TFlecsComponentHandle<T> RegisterComponentType(const bool bRegisterMemberProperties = true) const
	{
		solid_checkf(!IsDeferred(), TEXT("Cannot register component while deferred"));
		
		const FFlecsId AlreadyRegisteredId = GetIdIfRegistered<T>();
		
		// avoid calling RegisterMemberProperties if the component is already registered, as it would be redundant and potentially cause issues if the component was registered with different member properties settings
		if (AlreadyRegisteredId.IsValid())
		{
			return GetNativeFlecsWorld_Internal()->component<T>();
		}
		
		TFlecsComponentHandle<T> Component = GetNativeFlecsWorld_Internal()->component<T>();
		solid_check(Component.IsValid());
		
		if (bRegisterMemberProperties)
		{
			RegisterMemberProperties(TBaseStructure<T>::Get(), Component);
		}

		return Component;
	}

	/*template <typename T>
	FFlecsComponentHandle RegisterComponentType(const FString& InName, const bool bAllowTag = true, const FFlecsId InId = FFlecsId()) const
	{
		solid_checkf(!IsDeferred(), TEXT("Cannot register component while deferred"));

		FFlecsComponentHandle Component;

		World.component<T>(StringCast<char>(*InName).Get(), bAllowTag, InId);
		solid_check(Component.IsValid());

		if constexpr (Solid::IsScriptStruct<T>())
		{
			RegisterMemberProperties(TBaseStructure<T>::Get(), Component);
		}

		return Component;
	}*/
	
	FFlecsEntityHandle RegisterComponentType(const TSolidNotNull<const UScriptStruct*> ScriptStruct, const bool bRegisterMemberProperties = true) const;

	FFlecsEntityHandle RegisterComponentType(const TSolidNotNull<const UEnum*> ScriptEnum) const;
	
	UFUNCTION(BlueprintCallable, Category = "Flecs | World")
	FFlecsEntityHandle GetScriptStructEntity(const UScriptStruct* ScriptStruct) const;

	UFUNCTION(BlueprintCallable, Category = "Flecs | World")
	FFlecsEntityHandle GetScriptEnumEntity(const UEnum* ScriptEnum) const;
	
	UFUNCTION(BlueprintCallable, Category = "Flecs | World")
	bool HasScriptStruct(const UScriptStruct* ScriptStruct) const;

	UFUNCTION(BlueprintCallable, Category = "Flecs | World")
	bool HasScriptEnum(const UEnum* ScriptEnum) const;

	template <Solid::TScriptStructConcept T>
	NO_DISCARD FFlecsEntityHandle GetScriptStructEntity() const
	{
		return GetScriptStructEntity(TBaseStructure<T>::Get());
	}

	template <Solid::TScriptStructConcept T>
	NO_DISCARD bool HasScriptStruct() const
	{
		return HasScriptStruct(TBaseStructure<T>::Get());
	}

	template <Solid::TStaticEnumConcept T>
	NO_DISCARD FFlecsEntityHandle GetScriptEnumEntity() const
	{
		return GetScriptEnumEntity(StaticEnum<T>());
	}

	template <Solid::TStaticEnumConcept T>
	NO_DISCARD bool HasScriptEnum() const
	{
		return HasScriptEnum(StaticEnum<T>());
	}
	
	UFUNCTION(BlueprintCallable, Category = "Flecs")
	bool HasScriptClass(const TSubclassOf<UObject> InClass) const;

	UFUNCTION(BlueprintCallable, Category = "Flecs")
	FFlecsEntityHandle GetScriptClassEntity(const TSubclassOf<UObject> InClass) const;
	
	/**
	 * @brief Create a new entity in the world,
	 * currently if an entity of the same name exists, it will return the already existing entity
	 * @param Name Optional Name Parameter
	 * @return The created entity handle
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs | World", 
		meta = (AdvancedDisplay = "Separator, RootSeparator"))
	FFlecsEntityHandle CreateEntity(const FString& Name = "",
		const FString& Separator = "::", const FString& RootSeparator = "::") const;
	
	/**
	 * @brief Obtain a typed entity handle for the given Type
	 * @tparam T The type to obtain the entity for
	 * @return The entity obtained
	 */
	template <typename T>
	FFlecsEntityHandle ObtainTypedEntity() const
	{
		const FFlecsEntityHandle EntityHandle = GetNativeFlecsWorld_Internal()->entity<T>();
		return EntityHandle;
	}

	/**
	 * @brief 
	 * @param InClass The class to obtain the entity for
	 * @return The entity obtained
	 */
	FFlecsEntityHandle ObtainTypedEntity(const TSolidNotNull<UClass*> InClass) const;
	
	/**
	 * @brief Create a new entity in the world with the given Id, if the Id already exists, it will return the existing entity
	 * @param InId The Id to create the entity with
	 * @return The created entity handle
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs | World")
	FFlecsEntityHandle CreateEntityWithId(const FFlecsId InId) const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs | World")
	FFlecsEntityHandle CreateEntityWithRecord(const FFlecsEntityRecord& InRecord,
											  const FString& Name = "") const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs | World")
	FFlecsEntityHandle CreateEntityWithRecordWithId(const FFlecsEntityRecord& InRecord,
													const FFlecsId InId) const;
	
	template <typename TFunction>
	void WithScoped(const FFlecsId InId, TFunction&& Function) const
	{
		GetNativeFlecsWorld_Internal()->with(InId, std::forward<TFunction>(Function));
	}
	
	/**
	 * @brief Iterate over all Child Entities of the 0 Entity
	 * @tparam FunctionType The function type
	 * @param Function Function to invoke
	 */
	template <typename FunctionType>
	void ForEachChild(FunctionType&& Function) const
	{
		GetNativeFlecsWorld_Internal()->children(std::forward<FunctionType>(Function));
	}

	template <typename T, typename FunctionType>
	void ForEachChild(FunctionType&& Function) const
	{
		GetNativeFlecsWorld_Internal()->children<T>(std::forward<FunctionType>(Function));
	}

	template <typename FunctionType>
	void ForEachChild(const FFlecsId& InRelationId, FunctionType&& Function) const
	{
		GetNativeFlecsWorld_Internal()->children(InRelationId, std::forward<FunctionType>(Function));
	}
	
	/**
	 * @brief Destroy an entity by its handle, if the entity does not exist, nothing happens
	 * @param InName The name of the entity to destroy
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs | World")
	void DestroyEntityByName(const FString& InName) const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs")
	FFlecsEntityHandle GetTagEntity(const FGameplayTag& Tag) const;

	UFUNCTION(BlueprintCallable, Category = "Flecs")
	FFlecsEntityHandle CreatePrefabWithRecord(const FFlecsEntityRecord& InRecord, const FString& Name = "") const;

	UFUNCTION(BlueprintCallable, Category = "Flecs")
	FFlecsEntityHandle CreatePrefab(const FString& Name = "") const;

	template <typename T>
	FFlecsEntityHandle CreatePrefab() const
	{
		return GetNativeFlecsWorld_Internal()->prefab<T>();
	}

	FFlecsEntityHandle CreatePrefab(const TSolidNotNull<UClass*> InClass) const;
	FFlecsEntityHandle CreatePrefabWithRecord(const FFlecsEntityRecord& InRecord, const TSolidNotNull<UClass*> InClass) const;
	
	NO_DISCARD flecs::event_builder Event(const FFlecsEntityHandle& InEntity) const
	{
		return GetNativeFlecsWorld_Internal()->event(InEntity.GetEntity());
	}
	
	UFUNCTION()
	FFlecsQueryBuilder CreateQueryBuilder(const FString& InName = "") const;
	
	UFUNCTION()
	FFlecsQueryBuilder CreateQueryBuilderWithEntity(const FFlecsEntityHandle& InEntity) const;
	
	template <typename ...TComponents>
	NO_DISCARD TFlecsQueryBuilder<TComponents...> CreateQueryBuilder(const FString& InName = "")
	{
		return TFlecsQueryBuilder<TComponents...>(GetSelfInterface_Internal(), InName);
	}
	
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs")
	FFlecsQuery CreateQuery(const FFlecsQueryDefinition& InDefinition, const FString& InName = "") const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs")
	FFlecsQuery CreateQueryWithEntity(const FFlecsQueryDefinition& InDefinition, const FFlecsEntityHandle& InEntity) const;

	template <typename TEvent>
	flecs::event_builder_typed<TEvent> Event() const
	{
		return GetNativeFlecsWorld_Internal()->event<TEvent>();
	}
	
	template <typename ...TComponents>
	TFlecsObserverBuilder<TComponents...> CreateObserver(const FString& Name = "") const
	{
		return TFlecsObserverBuilder<TComponents...>(GetSelfInterface_Internal(), Name);
	}
	
	NO_DISCARD FORCEINLINE TFlecsObserverBuilder<> CreateObserverWithDefinition(
		const FFlecsObserverDefinition& InDefinition, const FString& Name = "") const
	{
		return TFlecsObserverBuilder<>(GetSelfInterface_Internal(), Name, InDefinition);
	}
	
	template <typename ...TComponents>
	NO_DISCARD TFlecsSystemBuilder<TComponents...> CreateSystem(const FString& InName = "") const
	{
		return TFlecsSystemBuilder<TComponents...>(GetSelfInterface_Internal(), InName);
	}
	
	NO_DISCARD TFlecsSystemBuilder<> CreateSystemWithDefinition(const FFlecsSystemDefinition& InDefinition, const FString& InName = "") const
	{
		return TFlecsSystemBuilder<>(this, InName, InDefinition);
	}
	
	NO_DISCARD flecs::pipeline_builder<> CreatePipeline() const
	{
		return GetNativeFlecsWorld_Internal()->pipeline();
	}

	template <typename ...TComponents>
	NO_DISCARD flecs::pipeline_builder<TComponents...> CreatePipeline() const
	{
		return GetNativeFlecsWorld_Internal()->pipeline<TComponents...>();
	}
	
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs")
	FFlecsTimerHandle CreateTimer(const FString& Name) const;

	template <typename T>
	NO_DISCARD FFlecsTimerHandle CreateTimer() const
	{
		return GetNativeFlecsWorld_Internal()->timer<T>();
	}
	
	/**
	 * @brief Equivalent to World.entity(flecs::World)
	 * @return The World Entity
	 */
	UFUNCTION(BlueprintCallable, Category = "Flecs | World")
	FFlecsEntityHandle GetWorldEntity() const;
	
	UFUNCTION(BlueprintCallable, Category = "Flecs | World")
	FFlecsEntityHandle GetPipeline() const;
	
	/**
	 * Get number of configured stages.
	 * Return number of stages set by set_stage_count().
	 *
	 * @return The number of stages used for threading.
	 *
	 * @see ecs_get_stage_count()
	 * @see flecs::world::set_stage_count()
	 */
	UFUNCTION(BlueprintCallable, Category = "Flecs | World")
	int32 GetStageCount() const;
	
	// @TODO: add missing function variations template and checked versions
	
	UFUNCTION(BlueprintCallable, Category = "Flecs | World")
	bool HasGameLoop(const TSubclassOf<UObject> InGameLoop, const bool bAllowChildren = false) const;
	
	UFUNCTION(BlueprintCallable, Category = "Flecs | World")
	FFlecsEntityHandle GetGameLoopEntity(const TSubclassOf<UObject> InGameLoop, const bool bAllowChildren = false) const;

	UFUNCTION(BlueprintCallable, Category = "Flecs | World")
	UObject* GetGameLoop(const TSubclassOf<UObject> InGameLoop, const bool bAllowChildren = false) const;
	
	UFUNCTION(BlueprintCallable, Category = "Flecs | World")
	int32 GetThreads() const;

	UFUNCTION(BlueprintCallable, Category = "Flecs | World")
	bool UsingTaskThreads() const;
	
	UFUNCTION(BlueprintCallable, Category = "Flecs")
	UObject* GetRegisteredFlecsObject(const TSubclassOf<UObject> InClass) const;
	
	template <Solid::TStaticClassConcept T>
	NO_DISCARD FORCEINLINE T* GetRegisteredFlecsObject() const
	{
		return Cast<T>(GetRegisteredFlecsObject(T::StaticClass()));
	}
	
	UFUNCTION(BlueprintCallable, Category = "Flecs")
	UObject* GetRegisteredFlecsObjectChecked(const TSubclassOf<UObject> InClass) const;
	
	template <Solid::TStaticClassConcept T>
	NO_DISCARD FORCEINLINE T* GetRegisteredFlecsObjectChecked() const
	{
		return CastChecked<T>(GetRegisteredFlecsObject(T::StaticClass()));
	}
	
	UFUNCTION(BlueprintCallable, Category = "Flecs")
	bool IsFlecsObjectRegistered(const TSubclassOf<UObject> InClass) const;
	
	template <Solid::TStaticClassConcept T>
	NO_DISCARD FORCEINLINE bool IsFlecsObjectRegistered() const
	{
		return IsFlecsObjectRegistered(T::StaticClass());
	}
	
	UFUNCTION(BlueprintCallable, Category = "Flecs")
	FFlecsEntityHandle GetFlecsModule(const FName& InModuleName) const;
	
protected:
	virtual flecs::world* GetNativeFlecsWorld_Internal() const PURE_VIRTUAL(UFlecsWorldInterfaceObject::GetNativeFlecsWorld_Internal, return nullptr;);
	
private:
	NO_DISCARD FORCEINLINE UFlecsWorldInterfaceObject* GetSelfInterface_Internal() const
	{
		return const_cast<UFlecsWorldInterfaceObject*>(this);
	}
	
}; // class UFlecsWorldInterfaceObject
