// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Net/Serialization/FastArraySerializer.h"

#include "FlecsReplicationLayoutDefinition.h"

#include "FlecsLayoutReplicatorFastArray.generated.h"

USTRUCT()
struct FFlecsLayoutReplicatorItem : public FFastArraySerializerItem
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	FFlecsReplicationLayoutDefinition LayoutDefinition;
	
}; // struct FFlecsLayoutReplicatorItem

USTRUCT()
struct FFlecsReplicatorFastArray : public FFastArraySerializer
{
	GENERATED_BODY()
	
public:
	
	void NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms);

	UPROPERTY()
	TArray<FFlecsLayoutReplicatorItem> Items;
	
}; // struct FFlecsReplicatorFastArray

template <>
struct TStructOpsTypeTraits<FFlecsReplicatorFastArray> : public TStructOpsTypeTraitsBase2<FFlecsReplicatorFastArray>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
	
}; // struct TStructOpsTypeTraits<FFlecsReplicatorFastArray>