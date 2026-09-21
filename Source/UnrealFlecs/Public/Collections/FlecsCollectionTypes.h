// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "Properties/FlecsComponentProperties.h"
#include "FlecsCollectionId.h"

#include "FlecsCollectionTypes.generated.h"

class UFlecsCollectionDataAsset;
class UFlecsCollectionClass;

/**
 * @brief Selects the source used to resolve a Collection reference.
 */
UENUM(BlueprintType)
enum class EFlecsCollectionReferenceMode : uint8
{
	/** Resolve the Collection from a UFlecsCollectionDataAsset. */
	Asset = 0,
	/** Resolve the Collection from a registered FFlecsCollectionId. */
	Id = 1,
	/** Resolve the Collection from a UClass implementing IFlecsCollectionInterface. */
	UClass = 2,

}; // enum class EFlecsCollectionReferenceMode

/**
 * @brief Identifies a Collection by asset, identifier, or interface class.
 *
 * Only the payload selected by Mode is used when the reference is resolved.
 * Use the FromAsset(), FromId(), or FromClass() factories to construct a
 * reference with the matching mode and payload.
 */
USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsCollectionReference
{
	GENERATED_BODY()

public:
	/** Creates an empty reference with Asset mode selected. */
	FORCEINLINE FFlecsCollectionReference() = default;

	/**
	 * @brief Creates a reference to a Collection data asset.
	 * @param InAsset Asset containing the Collection definition.
	 * @return A reference configured with Asset mode.
	 */
	static NO_DISCARD FFlecsCollectionReference FromAsset(const TSolidNotNull<const UFlecsCollectionDataAsset*> InAsset)
	{
		FFlecsCollectionReference Ref;
		Ref.Mode = EFlecsCollectionReferenceMode::Asset;
		Ref.Asset = InAsset;
		return Ref;
	}

	// @TODO: maybe validate param?
	/**
	 * @brief Creates a reference to a Collection interface class.
	 * @param InClass Class that implements IFlecsCollectionInterface.
	 * @return A reference configured with UClass mode.
	 */
	static NO_DISCARD FFlecsCollectionReference FromClass(const TSubclassOf<UObject> InClass)
	{
		FFlecsCollectionReference Ref;
		Ref.Mode = EFlecsCollectionReferenceMode::UClass;
		Ref.Class = InClass;
		return Ref;
	}

	/**
	 * @brief Creates a reference to a registered Collection identifier.
	 * @param InId Identifier used to find the registered Collection.
	 * @return A reference configured with Id mode.
	 */
	static NO_DISCARD FFlecsCollectionReference FromId(const FFlecsCollectionId& InId)
	{
		FFlecsCollectionReference Ref;
		Ref.Mode = EFlecsCollectionReferenceMode::Id;
		Ref.Id = InId;
		return Ref;
	}

	/**
	 * @brief Creates a reference from a string identifier.
	 * @param InIdString Name key of the registered Collection.
	 * @return A reference configured with Id mode.
	 */
	static NO_DISCARD FFlecsCollectionReference FromId(const FString& InIdString)
	{
		return FromId(FFlecsCollectionId(InIdString));
	}

	/** Resolution mode and source payload selected by this reference. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EFlecsCollectionReferenceMode Mode = EFlecsCollectionReferenceMode::Asset;

	/** Collection asset used when Mode is Asset. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly,
		meta = (EditCondition = "Mode == EFlecsCollectionReferenceMode::Asset", EditConditionHides))
	TObjectPtr<const UFlecsCollectionDataAsset> Asset;

	/** Collection interface class used when Mode is UClass. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly,
		meta = (EditCondition = "Mode == EFlecsCollectionReferenceMode::UClass", EditConditionHides,
			MustImplement = "/Script/UnrealFlecs.FlecsCollectionInterface"))
	TSubclassOf<UObject> Class;

	/** Registered Collection identifier used when Mode is Id. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly,
		meta = (EditCondition = "Mode == EFlecsCollectionReferenceMode::Id", EditConditionHides))
	FFlecsCollectionId Id;

}; // struct FFlecsCollectionReference

/**
 * @brief A Collection reference paired with parameters for instantiation.
 *
 * The parameters are passed to the Collection's parameter application
 * callback when the reference is added to an entity.
 */
USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsCollectionInstancedReference
{
	GENERATED_BODY()

public:
	/** Creates an empty instanced reference. */
	FORCEINLINE FFlecsCollectionInstancedReference() = default;

	/**
	 * @brief Creates an instanced reference from a Collection reference.
	 * @param InCollection Collection to resolve.
	 * @param InParameters Parameters supplied when the Collection is applied.
	 */
	FORCEINLINE FFlecsCollectionInstancedReference(
		const FFlecsCollectionReference& InCollection,
		const FInstancedStruct& InParameters = FInstancedStruct())
		: Collection(InCollection)
		, Parameters(InParameters)
	{
	}

	/** Collection reference to resolve. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flecs", meta = (ShowOnlyInnerProperties))
	FFlecsCollectionReference Collection;

	/** Optional parameters passed to the Collection during application. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flecs")
	FInstancedStruct Parameters;

}; // struct FFlecsCollectionInstancedReference

/**
 * @brief Stores Collection references that should be expanded on an entity.
 *
 * The Collection subsystem consumes this component while recursively
 * expanding references on Collection child entities.
 * A reference composes another Collection by adding `(IsA, Collection)`
 * during application and then removing the temporary reference.
 */
USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsCollectionReferenceComponent
{
	GENERATED_BODY()

	static constexpr bool Sparse = true;
	static constexpr flecs::on_instantiate OnInstantiate = flecs::on_instantiate::dont_inherit;

public:
	FORCEINLINE FFlecsCollectionReferenceComponent() = default;

	/** Collection references pending expansion on the owning entity. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flecs")
	TArray<FFlecsCollectionInstancedReference> Collections;

}; // struct FFlecsCollectionReferenceComponent

FLECS_COMPONENT_TRAITS(FFlecsCollectionReferenceComponent)
{
	static constexpr EFlecsOnInstantiate OnInstantiate = EFlecsOnInstantiate::DontInherit;

	static constexpr bool Sparse = true;
}; // struct FLECS_COMPONENT_TRAITS(FFlecsCollectionReferenceComponent)

/**
 * @brief Tags an entity as a registered Collection prefab.
 *
 * The Collection subsystem uses this tag to distinguish Collection prefabs
 * from arbitrary Flecs entities when resolving raw entity ids.
 */
USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsCollectionPrefabTag
{
	GENERATED_BODY()

	static constexpr flecs::on_instantiate OnInstantiate = flecs::on_instantiate::dont_inherit;

}; // struct FFlecsCollectionPrefabTag

FLECS_COMPONENT_TRAITS(FFlecsCollectionPrefabTag)
{
	static constexpr EFlecsOnInstantiate OnInstantiate = EFlecsOnInstantiate::DontInherit;

	static void PostRegister(const FFlecsComponentHandle& ComponentHandle)
	{
		ComponentHandle
			.AddPair(flecs::With, flecs::Prefab);
	}
};

/*USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsCollectionSlotTag
{
	GENERATED_BODY()
}; // struct FFlecsCollectionSlotTag

template <>
struct TFlecsComponentTraits<FFlecsCollectionSlotTag> : public TFlecsComponentTraitsBase<FFlecsCollectionSlotTag>
{
	static constexpr EFlecsOnInstantiate OnInstantiate = EFlecsOnInstantiate::DontInherit;
}; // struct TFlecsComponentTraits<FFlecsCollectionSlotTag>*/

// @TODO: Add Ordered Children
/**
 * @brief Stores the source index of a Collection sub-entity.
 */
USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsSubEntityIndex
{
	GENERATED_BODY()

	static constexpr bool Sparse = true;

	/** Index of the sub-entity in the owning FFlecsEntityRecord. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs")
	int32 Index = INDEX_NONE;
}; // struct FFlecsSubEntityIndex

FLECS_COMPONENT_TRAITS(FFlecsSubEntityIndex)
{
	static constexpr bool AutoRegister = false;

	static constexpr bool Sparse = true;
}; // struct FLECS_COMPONENT_TRAITS(FFlecsSubEntityIndex)

// @TODO: maybe add an OnSet Event like in templates
/**
 * @brief Describes how typed parameters are applied to a Collection instance.
 *
 * The component is stored on the Collection prefab. When the Collection is
 * added to an entity, the subsystem selects explicit parameters or the
 * stored default and invokes ApplyParametersFunction.
 */
USTRUCT()
struct UNREALFLECS_API FFlecsCollectionParametersComponent
{
	GENERATED_BODY()

	/** Callback that applies parameters to a target entity. */
	using FApplyParametersFunction = std::function<void(const FFlecsEntityHandle&, const FInstancedStruct&)>;

	static constexpr flecs::on_instantiate OnInstantiate = flecs::on_instantiate::dont_inherit;

public:
	/** Default parameter value and type expected by the Collection. */
	UPROPERTY()
	FInstancedStruct ParameterType;

	FApplyParametersFunction ApplyParametersFunction;

	/**
	 * @brief Stores a typed parameter application callback.
	 * @tparam T Script-struct type of the Collection parameters.
	 * @tparam FuncType Callable accepting a target entity and T.
	 * @param InFunction Callback invoked when the Collection is applied.
	 */
	template <Solid::TScriptStructConcept T, typename FuncType>
	void SetApplyParametersFunction(FuncType&& InFunction)
	{
		ApplyParametersFunction = [InFunction = SOLID_FWD(InFunction)]
			(const FFlecsEntityHandle& InEntityHandle, const FInstancedStruct& InParameters)
		{
			std::invoke(InFunction, InEntityHandle,
				TInstancedStruct<T>::InitializeAsScriptStruct(InParameters.GetScriptStruct(), InParameters.GetMemory()));
		};
	}

	/**
	 * @brief Applies parameters to a Collection instance.
	 * @param InEntityHandle Entity receiving the Collection.
	 * @param InParameters Parameters matching ParameterType.
	 * @warning The entity, parameters, and callback must all be valid.
	 */
	void ApplyParameters(const FFlecsEntityHandle& InEntityHandle, const FInstancedStruct& InParameters) const;

}; // struct FFlecsCollectionParametersComponent

FLECS_COMPONENT_TRAITS(FFlecsCollectionParametersComponent)
{
	static constexpr EFlecsOnInstantiate OnInstantiate = EFlecsOnInstantiate::DontInherit;
}; // struct FLECS_COMPONENT_TRAITS(FFlecsCollectionParametersComponent)

/*
USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsCollectionInstanceRelationship
{
	GENERATED_BODY()
}; // struct FFlecsCollectionInstanceRelationship

REGISTER_FLECS_COMPONENT(FFlecsCollectionInstanceRelationship,
	[](flecs::world InWorld, const FFlecsComponentHandle& InComponentHandle)
	{
		InComponentHandle
			.Add(flecs::DontFragment);
	});*/
