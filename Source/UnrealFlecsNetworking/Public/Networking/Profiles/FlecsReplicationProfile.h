// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once


#include "Properties/FlecsComponentProperties.h"

#include "FlecsReplicationProfile.generated.h"

struct FFlecsReplicationProfileParamsBase;

/** Flecs-owned policy values inherited by replicated entities through IsA. */
USTRUCT(BlueprintType)
struct UNREALFLECSNETWORKING_API FFlecsReplicationProfileDefinition
{
	GENERATED_BODY()
	
	// @TODO: make it only flecs components
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication")
	TArray<TInstancedStruct<FFlecsReplicationProfileParamsBase>> ParameterComponents;
	
	uint32 AddParam(const TInstancedStruct<FFlecsReplicationProfileParamsBase>& InParam);
	uint32 AddParam(const TSolidNotNull<const UScriptStruct*> InParamStruct);
	
	template <Solid::TScriptStructConcept T>
	requires (std::is_base_of_v<FFlecsReplicationProfileParamsBase, T>)
	FORCEINLINE uint32 AddParam(const T& InParam)
	{
		return AddParam(TInstancedStruct<T>::Make(InParam));
	}
	
	template <Solid::TScriptStructConcept T>
	requires (std::is_base_of_v<FFlecsReplicationProfileParamsBase, T>)
	FORCEINLINE uint32 AddParam()
	{
		return AddParam(TInstancedStruct<T>::Make({}));
	}
	
	FORCEINLINE bool operator==(const FFlecsReplicationProfileDefinition& Other) const
	{
		if (ParameterComponents.Num() != Other.ParameterComponents.Num())
		{
			return false;
		}
		
		for (int32 i = 0; i < ParameterComponents.Num(); ++i)
		{
			if (!ParameterComponents[i].IsValid() || !Other.ParameterComponents[i].IsValid())
			{
				return false;
			}
			
			if (ParameterComponents[i].GetScriptStruct() != Other.ParameterComponents[i].GetScriptStruct())
			{
				return false;
			}
			
			if (!ParameterComponents[i].GetScriptStruct()->CompareScriptStruct(ParameterComponents[i].GetMemory(), Other.ParameterComponents[i].GetMemory(), 0))
			{
				return false;
			}
		}
		
		return true;
	}
	
	FORCEINLINE bool operator!=(const FFlecsReplicationProfileDefinition& Other) const
	{
		return !(*this == Other);
	}
	
}; // struct FFlecsReplicationProfileDefinition

template <>
struct TFlecsComponentTraits<FFlecsReplicationProfileDefinition> : public TFlecsComponentTraitsBase<FFlecsReplicationProfileDefinition>
{
	static constexpr EFlecsOnInstantiate OnInstantiate = EFlecsOnInstantiate::Inherit;
}; // struct TFlecsComponentTraits<FFlecsReplicationProfileDefinition>

/** Identifies a Flecs entity as a replication profile prefab. */
USTRUCT(BlueprintType)
struct UNREALFLECSNETWORKING_API FFlecsReplicationProfileTag
{
	GENERATED_BODY()
}; // struct FFlecsReplicationProfileTag

template <>
struct TFlecsComponentTraits<FFlecsReplicationProfileTag> : public TFlecsComponentTraitsBase<FFlecsReplicationProfileTag>
{
	static constexpr EFlecsOnInstantiate OnInstantiate = EFlecsOnInstantiate::Inherit;
}; // struct TFlecsComponentTraits<FFlecsReplicationProfileTag>
