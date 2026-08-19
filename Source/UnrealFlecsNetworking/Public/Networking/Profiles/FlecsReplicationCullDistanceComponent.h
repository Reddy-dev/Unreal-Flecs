// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SolidMacros/Macros.h"

#include "Properties/FlecsComponentProperties.h"

#include "FlecsReplicationCullDistanceComponent.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECSNETWORKING_API FFlecsReplicationCullDistanceComponent
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication")
	float CullDistance = 0.f;

	NO_DISCARD bool operator==(const FFlecsReplicationCullDistanceComponent& Other) const
	{
		return CullDistance == Other.CullDistance;
	}

	NO_DISCARD bool operator!=(const FFlecsReplicationCullDistanceComponent& Other) const
	{
		return !(*this == Other);
	}
	
}; // struct FFlecsReplicationProfileCullDistance

template <>
struct TIsPODType<FFlecsReplicationCullDistanceComponent>
{
	enum { Value = true };
}; // struct TIsPODType<FFlecsReplicationCullDistanceComponent

template <>
struct TFlecsComponentTraits<FFlecsReplicationCullDistanceComponent> : public TFlecsComponentTraitsBase<FFlecsReplicationCullDistanceComponent>
{
	static constexpr EFlecsOnInstantiate OnInstantiate = EFlecsOnInstantiate::Inherit;
}; // struct TFlecsComponentTraits<FFlecsReplicationProfileCullDistance>
