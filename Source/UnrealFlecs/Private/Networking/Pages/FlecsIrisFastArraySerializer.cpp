// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Pages/FlecsIrisFastArraySerializer.h"

#include "Networking/Pages/FlecsNetEntityPage.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsIrisFastArraySerializer)

void FFlecsNetEntityPageArray::SetOwner(const TSolidNotNull<UFlecsNetEntityPage*> InOwner)
{
	solid_check(IsValid(InOwner));
	Owner = InOwner;
}

bool FFlecsNetEntityPageArray::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FFlecsNetEntityPageItem, FFlecsNetEntityPageArray>(
		Items, DeltaParms, *this);
}

void FFlecsNetEntityPageArray::PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize)
{
	
}

void FFlecsNetEntityPageArray::PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize)
{
}

void FFlecsNetEntityPageArray::PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize)
{
}
