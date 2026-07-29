// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Net/Serialization/FastArraySerializer.h"

#include "FlecsReplicationLayoutDefinition.h"

#include "FlecsLayoutReplicatorFastArray.generated.h"

class UFlecsLayoutReplicator;

struct FFlecsReplicatorFastArray;

USTRUCT()
struct UNREALFLECS_API FFlecsLayoutReplicatorItem : public FFastArraySerializerItem
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	FFlecsReplicationLayoutDefinition LayoutDefinition;

	void PostReplicatedAdd(const FFlecsReplicatorFastArray& InArraySerializer);
	void PostReplicatedChange(const FFlecsReplicatorFastArray& InArraySerializer);
	
}; // struct FFlecsLayoutReplicatorItem

USTRUCT()
struct UNREALFLECS_API FFlecsReplicatorFastArray : public FFastArraySerializer
{
	GENERATED_BODY()
	
public:
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms);

	void SetOwner(UFlecsLayoutReplicator* InOwner);

	/** Adds an immutable layout, or leaves an identical retained definition untouched. */
	NO_DISCARD bool AddLayout(const FFlecsReplicationLayoutDefinition& InLayoutDefinition);

	NO_DISCARD const FFlecsLayoutReplicatorItem* FindLayout(
		const FFlecsReplicationLayoutId& InLayoutId) const;

	UPROPERTY()
	TArray<FFlecsLayoutReplicatorItem> Items;

private:
	friend struct FFlecsLayoutReplicatorItem;

	void ReceiveLayout(const FFlecsReplicationLayoutDefinition& InLayoutDefinition) const;

	UFlecsLayoutReplicator* Owner = nullptr;
	
}; // struct FFlecsReplicatorFastArray

template <>
struct TStructOpsTypeTraits<FFlecsReplicatorFastArray> : public TStructOpsTypeTraitsBase2<FFlecsReplicatorFastArray>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
	
}; // struct TStructOpsTypeTraits<FFlecsReplicatorFastArray>
