// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Iris/ReplicationState/IrisFastArraySerializer.h"

#include "Networking/FlecsNetworkId.h"
#include "Networking/Layout/FlecsReplicationSnapshot.h"

#include "FlecsNetEntityTableArray.generated.h"

class UFlecsNetEntityTable;

/** One replicated entity record stored in a table-backed shard. */
USTRUCT()
struct UNREALFLECSNETWORKING_API FFlecsNetEntityTableItem : public FFastArraySerializerItem
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FFlecsNetworkId NetworkId;

	UPROPERTY()
	FFlecsEntityReplicationSnapshot Snapshot;

}; // struct FFlecsNetEntityTableItem

/** Iris Fast Array payload owned by a table-backed shard. */
USTRUCT()
struct UNREALFLECSNETWORKING_API FFlecsNetEntityTableArray : public FIrisFastArraySerializer
{
	GENERATED_BODY()

public:
	FFlecsNetEntityTableArray()
		: Owner(nullptr)
	{
	}

	void SetOwner(const TSolidNotNull<UFlecsNetEntityTable*> InOwner);

	UPROPERTY()
	TArray<FFlecsNetEntityTableItem> Items;

	UPROPERTY(Transient, NotReplicated)
	TWeakObjectPtr<UFlecsNetEntityTable> Owner;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms);

	void PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize);

}; // struct FFlecsNetEntityTableArray

template<>
struct TStructOpsTypeTraits<FFlecsNetEntityTableArray> : public TStructOpsTypeTraitsBase2<FFlecsNetEntityTableArray>
{
	enum
	{
		WithNetDeltaSerializer = true
	};
	
}; // struct TStructOpsTypeTraits<FFlecsNetEntityTableArray>
