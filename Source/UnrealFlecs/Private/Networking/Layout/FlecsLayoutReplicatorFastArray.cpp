// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Layout/FlecsLayoutReplicatorFastArray.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsLayoutReplicatorFastArray)

void FFlecsReplicatorFastArray::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
{
	FFastArraySerializer::FastArrayDeltaSerialize<FFlecsLayoutReplicatorItem, FFlecsReplicatorFastArray>(
		Items, DeltaParms, *this);
}
