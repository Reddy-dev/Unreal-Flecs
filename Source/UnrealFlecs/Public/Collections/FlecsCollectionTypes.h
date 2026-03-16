// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Properties/FlecsComponentProperties.h"
#include "FlecsCollectionId.h"

#include "FlecsCollectionTypes.generated.h"

class UFlecsCollectionDataAsset;
class UFlecsCollectionClass;

UENUM(BlueprintType)
enum class EFlecsCollectionReferenceMode : uint8
{
	Asset = 0,
	Id = 1,
	UClass = 2,
	
}; // enum class EFlecsCollectionReferenceMode

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsCollectionReference
{
	GENERATED_BODY()

public:
	FORCEINLINE FFlecsCollectionReference() = default;

	static NO_DISCARD FFlecsCollectionReference FromAsset(const TSolidNotNull<const UFlecsCollectionDataAsset*> InAsset)
	{
		FFlecsCollectionReference Ref;
		Ref.Mode = EFlecsCollectionReferenceMode::Asset;
		Ref.Asset = InAsset;
		return Ref;
	}

	// @TODO: maybe validate param?
	static NO_DISCARD FFlecsCollectionReference FromClass(const TSubclassOf<UObject> InClass)
	{
		FFlecsCollectionReference Ref;
		Ref.Mode = EFlecsCollectionReferenceMode::UClass;
		Ref.Class = InClass;
		return Ref;
	}

	static NO_DISCARD FFlecsCollectionReference FromId(const FFlecsCollectionId& InId)
	{
		FFlecsCollectionReference Ref;
		Ref.Mode = EFlecsCollectionReferenceMode::Id;
		Ref.Id = InId;
		return Ref;
	}

	static NO_DISCARD FFlecsCollectionReference FromId(const FString& InIdString)
	{
		return FromId(FFlecsCollectionId(InIdString));
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flecs")
	EFlecsCollectionReferenceMode Mode = EFlecsCollectionReferenceMode::Asset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flecs",
		meta = (EditCondition = "Mode == EFlecsCollectionReferenceMode::Asset", EditConditionHides))
	TObjectPtr<const UFlecsCollectionDataAsset> Asset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flecs",
		meta = (EditCondition = "Mode == EFlecsCollectionReferenceMode::UClass", EditConditionHides,
			MustImplement = "/Script/UnrealFlecs.FlecsCollectionInterface"))
	TSubclassOf<UObject> Class;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flecs",
		meta = (EditCondition = "Mode == EFlecsCollectionReferenceMode::Id", EditConditionHides))
	FFlecsCollectionId Id;
	
}; // struct FFlecsCollectionReference
USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsCollectionInstancedReference
{
	GENERATED_BODY()

public:
	FORCEINLINE FFlecsCollectionInstancedReference() = default;

	FORCEINLINE FFlecsCollectionInstancedReference(
			const FFlecsCollectionReference& InCollection, const FInstancedStruct& InParameters = FInstancedStruct())
		: Collection(InCollection)
		, Parameters(InParameters)
	{
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flecs", meta = (ShowOnlyInnerProperties))
	FFlecsCollectionReference Collection;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flecs")
	FInstancedStruct Parameters;
	
}; // struct FFlecsCollectionInstancedReference

/** compose another Collection by reference (adds (IsA, @param Collection) in compile-time and then removes itself). */
USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsCollectionReferenceComponent
{
	GENERATED_BODY()

public:
	FORCEINLINE FFlecsCollectionReferenceComponent() = default;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flecs")
	TArray<FFlecsCollectionInstancedReference> Collections;
	
}; // struct FFlecsCollectionReferenceComponent

template <>
struct TFlecsComponentTraits<FFlecsCollectionReferenceComponent> : public TFlecsComponentTraitsBase<FFlecsCollectionReferenceComponent>
{
	static constexpr EFlecsOnInstantiate OnInstantiate = EFlecsOnInstantiate::DontInherit;
	static constexpr bool Sparse = true;
}; // struct TFlecsComponentTraits<FFlecsCollectionReferenceComponent>

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsCollectionPrefabTag
{
	GENERATED_BODY()
}; // struct FFlecsCollectionPrefabTag

template <>
struct TFlecsComponentTraits<FFlecsCollectionPrefabTag> : public TFlecsComponentTraitsBase<FFlecsCollectionPrefabTag>
{
	static constexpr EFlecsOnInstantiate OnInstantiate = EFlecsOnInstantiate::DontInherit;
	
	static void PostRegister(const FFlecsComponentHandle& ComponentHandle)
	{
		ComponentHandle.AddPair(flecs::With, flecs::Prefab);
	}
};

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsCollectionSlotTag
{
	GENERATED_BODY()
}; // struct FFlecsCollectionSlotTag

template <>
struct TFlecsComponentTraits<FFlecsCollectionSlotTag> : public TFlecsComponentTraitsBase<FFlecsCollectionSlotTag>
{
	static constexpr EFlecsOnInstantiate OnInstantiate = EFlecsOnInstantiate::DontInherit;
}; // struct TFlecsComponentTraits<FFlecsCollectionSlotTag>

// @TODO: Add Ordered Children
USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsSubEntityIndex
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs")
	int32 Index = INDEX_NONE;
}; // struct FFlecsSubEntityIndex

template <>
struct TFlecsComponentTraits<FFlecsSubEntityIndex> : public TFlecsComponentTraitsBase<FFlecsSubEntityIndex>
{
	static constexpr bool Sparse = true;
}; // struct TFlecsComponentTraits<FFlecsSubEntityIndex>

// @TODO: maybe add an OnSet Event like in templates

USTRUCT()
struct UNREALFLECS_API FFlecsCollectionParametersComponent
{
	GENERATED_BODY()

	using FApplyParametersFunction = std::function<void(const FFlecsEntityHandle&, const FInstancedStruct&)>;

public:
	UPROPERTY()
	FInstancedStruct ParameterType;

	FApplyParametersFunction ApplyParametersFunction;

	template <Solid::TScriptStructConcept T, typename FuncType>
	void SetApplyParametersFunction(FuncType&& InFunction)
	{
		ApplyParametersFunction = [InFunction = std::forward<FuncType>(InFunction)]
			(const FFlecsEntityHandle& InEntityHandle, const FInstancedStruct& InParameters)
		{
			std::invoke(InFunction, InEntityHandle,
				TInstancedStruct<T>::InitializeAsScriptStruct(InParameters.GetScriptStruct(), InParameters.GetMemory()));
		};
	}

	void ApplyParameters(const FFlecsEntityHandle& InEntityHandle, const FInstancedStruct& InParameters) const;
	
}; // struct FFlecsCollectionParametersComponent

template <>
struct TFlecsComponentTraits<FFlecsCollectionParametersComponent> : public TFlecsComponentTraitsBase<FFlecsCollectionParametersComponent>
{	
	static constexpr EFlecsOnInstantiate OnInstantiate = EFlecsOnInstantiate::DontInherit;
}; // struct TFlecsComponentTraits<FFlecsCollectionParametersComponent>

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