// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "FlecsCollectionBuilder.h"
#include "FlecsCollectionDefinition.h"
#include "FlecsCollectionId.h"

#include "Worlds/FlecsAbstractWorldSubsystem.h"

#include "FlecsCollectionWorldSubsystem.generated.h"

class UFlecsCollectionWorldSubsystem;
class IFlecsCollectionInterface;
class UFlecsCollectionDataAsset;

namespace UE::Flecs::Collections
{
	template <typename FuncType>
	/**
	 * @brief Constraint for callbacks that configure a Collection builder.
	 * @tparam FuncType Callable accepting FFlecsCollectionBuilder& and returning void.
	 */
	concept TCollectionBuilderFunc = requires(FuncType Func, FFlecsCollectionBuilder& Builder)
	{
		{ Func(Builder) } -> std::same_as<void>;
	}; // concept TCollectionBuilderFunc
	
} // namespace UE::Flecs::Collections

USTRUCT()
/**
 * @brief Singleton that exposes the owning Collection subsystem to entity APIs.
 */
struct UNREALFLECS_API FFlecsCollectionSubsystemSingleton
{
	GENERATED_BODY()

public:
	UPROPERTY()
	/** World subsystem used to register and apply Collections. */
	TWeakObjectPtr<UFlecsCollectionWorldSubsystem> WorldSubsystem;
	
}; // struct FFlecsCollectionSubsystemSingleton

template <>
struct TFlecsComponentTraits<FFlecsCollectionSubsystemSingleton> : public TFlecsComponentTraitsBase<FFlecsCollectionSubsystemSingleton>
{
	static constexpr bool Singleton = true;
}; // struct TFlecsComponentTraits<FFlecsCollectionSubsystemSingleton>

UCLASS()
/**
 * @brief Registers Collection definitions and applies them as Flecs prefabs.
 *
 * The subsystem owns the mapping from FFlecsCollectionId values to prefab
 * entities. Registering a Collection resolves its record and nested
 * references into a prefab; applying one adds an IsA relationship to a target
 * entity and invokes its optional parameter callback.
 */
class UNREALFLECS_API UFlecsCollectionWorldSubsystem final : public UFlecsAbstractWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Creates the Collection subsystem. */
	UFlecsCollectionWorldSubsystem();
	
	/** Initializes the Unreal subsystem before its Flecs world is ready. */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	/** Connects the subsystem to its initialized Flecs world. */
	virtual void OnFlecsWorldInitialized(const TSolidNotNull<UFlecsWorld*> InWorld) override;
	/** Releases subsystem state during world teardown. */
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Flecs|Collections")
	/**
	 * @brief Registers a data asset as a Collection prefab.
	 * @param InAsset Asset containing the Collection definition.
	 * @return The registered Collection prefab entity.
	 *
	 * The asset name is used as the Collection identifier.
	 */
	FFlecsEntityHandle RegisterCollectionAsset(const UFlecsCollectionDataAsset* InAsset);

	/**
	 * @brief Registers an Unreal-owned Collection definition.
	 * @param InName Name key used to identify the Collection.
	 * @param InDefinition Definition converted into a Flecs prefab.
	 * @return The registered Collection prefab entity.
	 */
	FFlecsEntityHandle RegisterCollectionDefinition(const FString& InName, FFlecsCollectionDefinition& InDefinition);
	/**
	 * @brief Registers a Collection definition under a UObject class.
	 * @param InClass Class used as the Collection's identity and prefab name.
	 * @param InBuilder Builder containing the class's Collection definition.
	 * @return The registered Collection prefab entity.
	 * @warning InClass must not implement IFlecsCollectionInterface; use
	 * RegisterCollectionInterfaceClass() for interface-backed classes.
	 */
	FFlecsEntityHandle RegisterCollectionClass(const TSolidNotNull<UClass*> InClass, const FFlecsCollectionBuilder& InBuilder);
	/**
	 * @brief Builds and registers a Collection from a class default object.
	 * @param InInterfaceObject UObject class implementing
	 * IFlecsCollectionInterface.
	 * @return The registered Collection prefab entity.
	 */
	FFlecsEntityHandle RegisterCollectionInterfaceClass(const TSolidNotNull<UClass*> InInterfaceObject);

	template <Solid::TStaticClassConcept T>
	/**
	 * @brief Builds and registers a typed Collection interface class.
	 * @tparam T UObject class implementing IFlecsCollectionInterface.
	 * @return The registered Collection prefab entity.
	 */
	FORCEINLINE FFlecsEntityHandle RegisterCollectionInterfaceClass()
	{
		static_assert(TIsDerivedFrom<T, IFlecsCollectionInterface>::Value,
		              "T must implement IFlecsCollectionInterface");
		return RegisterCollectionInterfaceClass(T::StaticClass());
	}
	
	UFUNCTION(BlueprintCallable, Category = "Flecs|Collections")
	/** Returns the prefab registered for a Collection data asset. */
	FFlecsEntityHandle GetPrefabByAsset(const UFlecsCollectionDataAsset* Asset) const;

	UFUNCTION(BlueprintCallable, Category = "Flecs|Collections")
	/**
	 * @brief Resolves a raw Flecs id if it identifies a Collection prefab.
	 * @param Id Native Flecs id of the candidate prefab.
	 * @return The Collection prefab handle, or an invalid handle.
	 */
	FFlecsEntityHandle GetPrefabByIdRaw(const FFlecsId& Id) const;

	UFUNCTION(BlueprintCallable, Category = "Flecs|Collections")
	/**
	 * @brief Finds a registered Collection prefab by name identifier.
	 * @param Id Collection identifier used for lookup.
	 * @return The Collection prefab handle, or an invalid handle.
	 */
	FFlecsEntityHandle GetPrefabByCollectionId(const FFlecsCollectionId& Id) const;

	UFUNCTION(BlueprintCallable, Category = "Flecs|Collections")
	/**
	 * @brief Finds the prefab registered for a Collection class.
	 * @param InClass Class used when the Collection was registered.
	 * @return The Collection prefab handle, or an invalid handle.
	 */
	FFlecsEntityHandle GetPrefabByClass(const TSubclassOf<UObject> InClass) const;

	template <UE::Flecs::Collections::TCollectionBuilderFunc FuncType>
	/**
	 * @brief Builds and registers a Collection from a callback.
	 * @tparam FuncType Callable that configures an FFlecsCollectionBuilder.
	 * @param InBuildFunc Builder callback.
	 * @return The registered Collection prefab entity.
	 */
	FFlecsEntityHandle RegisterCollectionBuilder(FuncType&& InBuildFunc)
	{
		FFlecsCollectionDefinition Definition;

		FFlecsCollectionBuilder Builder = FFlecsCollectionBuilder::Create(Definition);

		std::invoke(SOLID_FWD(InBuildFunc), Builder);

		return RegisterCollectionDefinition(Builder.IdName, Definition);
	}

	template <UE::Flecs::Collections::TCollectionBuilderFunc FuncType>
	/**
	 * @brief Builds and registers a class-owned Collection from a callback.
	 * @tparam FuncType Callable that configures an FFlecsCollectionBuilder.
	 * @param InClass Class used as the Collection identity.
	 * @param InBuildFunc Builder callback.
	 * @return The registered Collection prefab entity.
	 */
	FFlecsEntityHandle RegisterCollectionClass(const TSolidNotNull<UClass*> InClass, FuncType&& InBuildFunc)
	{
		checkf(!ClassImplementsCollectionInterface(InClass),
		       TEXT("Use RegisterCollectionInterfaceClass to register collection classes that implement IFlecsCollectionInterface"));
		
		FFlecsCollectionDefinition Definition;
		
		FFlecsCollectionBuilder Builder = FFlecsCollectionBuilder::Create(Definition);
		std::invoke(SOLID_FWD(InBuildFunc), Builder);
		
		return RegisterCollectionClass(InClass, Builder);
	}

	/**
	 * @brief Adds a Collection prefab identified by a raw Flecs id.
	 * @param InEntity Target entity receiving the Collection.
	 * @param InCollectionId Raw id of a registered Collection prefab.
	 * @param InParameters Explicit parameters, or the Collection default when invalid.
	 */
	void AddCollectionToEntity(const FFlecsEntityHandle& InEntity, const FFlecsId InCollectionId,
		const FInstancedStruct& InParameters = FInstancedStruct());
	
	/**
	 * @brief Adds a registered Collection by Collection identifier.
	 * @param InEntity Target entity receiving the Collection.
	 * @param InCollectionId Collection identifier used for lookup.
	 * @param InParameters Explicit parameters, or the Collection default when invalid.
	 */
	void AddCollectionToEntity(const FFlecsEntityHandle& InEntity, const FFlecsCollectionId& InCollectionId,
		const FInstancedStruct& InParameters = FInstancedStruct());
	
	/**
	 * @brief Adds a Collection resolved from a reference.
	 * @param InEntity Target entity receiving the Collection.
	 * @param InCollectionReference Asset, id, or interface-class reference.
	 * @param InParameters Explicit parameters, or the Collection default when invalid.
	 */
	void AddCollectionToEntity(const FFlecsEntityHandle& InEntity, const FFlecsCollectionReference& InCollectionReference,
		const FInstancedStruct& InParameters = FInstancedStruct());
	
	/** Adds the Collection described by InAsset to InEntity. */
	void AddCollectionToEntity(const FFlecsEntityHandle& InEntity, const TSolidNotNull<const UFlecsCollectionDataAsset*> InAsset,
		const FInstancedStruct& InParameters = FInstancedStruct());
	
	/** Adds the Collection implemented by InClass to InEntity. */
	void AddCollectionToEntity(const FFlecsEntityHandle& InEntity, const TSubclassOf<UObject> InClass,
		const FInstancedStruct& InParameters = FInstancedStruct());

	template <Solid::TStaticClassConcept T>
	/**
	 * @brief Adds a typed interface Collection to an entity.
	 * @tparam T UObject class implementing IFlecsCollectionInterface.
	 * @param InEntity Target entity receiving the Collection.
	 */
	FORCEINLINE void AddCollectionToEntity(const FFlecsEntityHandle& InEntity)
	{
		static_assert(TIsDerivedFrom<T, IFlecsCollectionInterface>::Value,
		              "T must implement IFlecsCollectionInterface");
		AddCollectionToEntity(InEntity, T::StaticClass());
	}
	
	/**
	 * @brief Removes a Collection identified by Collection id.
	 * @param InEntity Target entity.
	 * @param InCollectionId Collection identifier used for lookup.
	 *
	 * Removing the IsA relationship does not remove components overridden on
	 * the target entity.
	 */
	void RemoveCollectionFromEntity(const FFlecsEntityHandle& InEntity, const FFlecsCollectionId& InCollectionId);
	/** Removes the Collection resolved from InCollectionReference. */
	void RemoveCollectionFromEntity(const FFlecsEntityHandle& InEntity, const FFlecsCollectionReference& InCollectionReference);
	/** Removes the Collection described by InAsset from InEntity. */
	void RemoveCollectionFromEntity(const FFlecsEntityHandle& InEntity, const TSolidNotNull<const UFlecsCollectionDataAsset*> InAsset);
	/** Removes the Collection implemented by InClass from InEntity. */
	void RemoveCollectionFromEntity(const FFlecsEntityHandle& InEntity, const TSubclassOf<UObject> InClass);
	/** Removes a Collection prefab identified by a raw Flecs id. */
	void RemoveCollectionFromEntity(const FFlecsEntityHandle& InEntity, const FFlecsId InCollectionId);

	template <Solid::TStaticClassConcept T>
	/**
	 * @brief Removes a typed interface Collection from an entity.
	 * @tparam T UObject class implementing IFlecsCollectionInterface.
	 * @param InEntity Target entity.
	 */
	FORCEINLINE void RemoveCollectionFromEntity(const FFlecsEntityHandle& InEntity)
	{
		static_assert(TIsDerivedFrom<T, IFlecsCollectionInterface>::Value,
		              "T must implement IFlecsCollectionInterface");
		RemoveCollectionFromEntity(InEntity, T::StaticClass());
	}

	
	/**
	 * @brief Tests whether an entity inherits a registered Collection.
	 * @param InEntity Entity to inspect.
	 * @param InCollectionId Collection identifier used for lookup.
	 * @return True when the entity has an IsA relationship to the Collection.
	 */
	NO_DISCARD bool HasCollection(const FFlecsEntityHandle& InEntity, const FFlecsCollectionId& InCollectionId) const;

	/** Returns whether a Collection identifier has been registered. */
	NO_DISCARD bool IsCollectionRegistered(const FFlecsCollectionId& Id) const;

private:

	// Create/Find the collection entity referenced in the @Reference
	NO_DISCARD FFlecsEntityHandle ResolveCollectionReference(const FFlecsCollectionReference& Reference);

	// Make sure any Collection References Child Entities may have of this collection are cleaned up/created (is called recursively)
	void ExpandChildCollectionReferences(const FFlecsEntityHandle& InCollectionEntity);

	NO_DISCARD FFlecsEntityHandle CreatePrefabEntity(const FString& Name, const FFlecsEntityRecord& Record) const;
	NO_DISCARD FFlecsEntityHandle CreatePrefabEntity(const TSolidNotNull<UClass*> InClass,
		const FFlecsEntityRecord& Record) const;

	void ApplyCollectionParametersToEntity(const FFlecsEntityHandle& InEntity,
		const FFlecsEntityHandle& InCollectionEntity,
		const FInstancedStruct& InParameters) const;

	void ApplyNamesToSubEntities(FFlecsCollectionDefinition& InDefinition) const;
	
	NO_DISCARD bool ClassImplementsCollectionInterface(const TSolidNotNull<const UClass*> InClass) const;

	UPROPERTY()
	TMap<FFlecsCollectionId, FFlecsEntityView> RegisteredCollections;

	// @TODO: make use of this
	// Recursion guard
	TSet<FFlecsCollectionId> InProgressCollections;
	
}; // class UFlecsCollectionWorldSubsystem
