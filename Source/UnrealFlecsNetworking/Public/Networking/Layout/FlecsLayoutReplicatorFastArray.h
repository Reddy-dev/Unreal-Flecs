// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsReplicationLayoutDefinition.h"
#include "Iris/ReplicationState/IrisFastArraySerializer.h"

#include "FlecsLayoutReplicatorFastArray.generated.h"

class UFlecsIrisReplicationBridge;

struct FFlecsReplicatorFastArray;

USTRUCT()
struct UNREALFLECSNETWORKING_API FFlecsLayoutReplicatorItem : public FFastArraySerializerItem
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	FFlecsReplicationLayoutDefinition LayoutDefinition;

	void PostReplicatedAdd(const FFlecsReplicatorFastArray& InArraySerializer);
	void PostReplicatedChange(const FFlecsReplicatorFastArray& InArraySerializer);
	
}; // struct FFlecsLayoutReplicatorItem

USTRUCT()
struct UNREALFLECSNETWORKING_API FFlecsReplicatorFastArray : public FIrisFastArraySerializer
{
	GENERATED_BODY()
	
public:
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms);

	void SetOwner(UFlecsIrisReplicationBridge* InOwner);

	/** Adds an immutable layout, or leaves an identical retained definition untouched. */
	bool AddLayout(const FFlecsReplicationLayoutDefinition& InLayoutDefinition);

	NO_DISCARD const FFlecsLayoutReplicatorItem* FindLayout(const FFlecsReplicationLayoutId& InLayoutId) const;

	UPROPERTY()
	TArray<FFlecsLayoutReplicatorItem> Items;

private:
	friend struct FFlecsLayoutReplicatorItem;

	void ReceiveLayout(const FFlecsReplicationLayoutDefinition& InLayoutDefinition) const;

	UPROPERTY(Transient, NotReplicated)
	TWeakObjectPtr<UFlecsIrisReplicationBridge> Owner;
	
}; // struct FFlecsReplicatorFastArray

template <>
struct TStructOpsTypeTraits<FFlecsReplicatorFastArray> : public TStructOpsTypeTraitsBase2<FFlecsReplicatorFastArray>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
	
}; // struct TStructOpsTypeTraits<FFlecsReplicatorFastArray>
