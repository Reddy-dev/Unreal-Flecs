// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once


#include "Properties/FlecsComponentProperties.h"

#include "FlecsReplicationUpdateRateComponent.generated.h"

USTRUCT()
struct FFlecsReplicationUpdateRateComponent
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = "Replication")
	float UpdateRate = 0.f;
	
	NO_DISCARD bool operator==(const FFlecsReplicationUpdateRateComponent& Other) const
	{
		return UpdateRate == Other.UpdateRate;
	}
	
	NO_DISCARD bool operator!=(const FFlecsReplicationUpdateRateComponent& Other) const
	{
		return !(*this == Other);
	}
	
}; // struct FFlecsReplicationUpdateRateComponent

template <>
struct TFlecsComponentTraits<FFlecsReplicationUpdateRateComponent> : public TFlecsComponentTraitsBase<FFlecsReplicationUpdateRateComponent>
{
	static constexpr EFlecsOnInstantiate OnInstantiate = EFlecsOnInstantiate::Inherit;
}; // struct TFlecsComponentTraits<FFlecsReplicationUpdateRateComponent>

template <>
struct TStructOpsTypeTraits<FFlecsReplicationUpdateRateComponent> : public TStructOpsTypeTraitsBase2<FFlecsReplicationUpdateRateComponent>
{
	enum
	{
		WithCopy = true,
		WithMoveAssign = true,
	}; // enum
	
}; // struct TStructOpsTypeTraitsBase<FFlecsReplicationCullDistanceComponent>
