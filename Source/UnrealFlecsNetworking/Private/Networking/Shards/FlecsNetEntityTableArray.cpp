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

void FFlecsNetEntityTableArray::PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize)
{
}

void FFlecsNetEntityTableArray::PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize)
{
}

void FFlecsNetEntityTableArray::PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize)
{
}
