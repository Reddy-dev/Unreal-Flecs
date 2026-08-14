// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Shards/FlecsNetEntityTableArray.h"

#include "Networking/Shards/FlecsNetEntityTable.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsNetEntityTableArray)

void FFlecsNetEntityTableArray::SetOwner(const TSolidNotNull<UFlecsNetEntityTable*> InOwner)
{
	solid_check(IsValid(InOwner));
	Owner = InOwner;
}

bool FFlecsNetEntityTableArray::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FFlecsNetEntityTableItem, FFlecsNetEntityTableArray>(
		Items, DeltaParms, *this);
}

void FFlecsNetEntityTableArray::PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32)
{
	UFlecsNetEntityTable* Table = Owner.Get();
	if UNLIKELY_IF(!Table)
	{
		return;
	}

	for (const int32 RemovedIndex : RemovedIndices)
	{
		if (!Items.IsValidIndex(RemovedIndex))
		{
			continue;
		}

		const FFlecsNetEntityTableItem& Item = Items[RemovedIndex];
		Table->HandleEntityRemoved(Item.NetworkId, Item.Snapshot.StateRevision);
	}
}

void FFlecsNetEntityTableArray::PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize)
{
	PostReplicatedChange(AddedIndices, FinalSize);
}

void FFlecsNetEntityTableArray::PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32)
{
	UFlecsNetEntityTable* Table = Owner.Get();
	if UNLIKELY_IF(!Table)
	{
		return;
	}

	for (const int32 ChangedIndex : ChangedIndices)
	{
		if (!Items.IsValidIndex(ChangedIndex))
		{
			continue;
		}

		const FFlecsNetEntityTableItem& Item = Items[ChangedIndex];
		Table->HandleEntityUpdated(Item.NetworkId, Item.Snapshot);
	}
}
