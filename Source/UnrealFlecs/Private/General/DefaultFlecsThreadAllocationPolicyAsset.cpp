// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "General/DefaultFlecsThreadAllocationPolicyAsset.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DefaultFlecsThreadAllocationPolicyAsset)

TTuple<EFlecsThreadAllocationType, int32> UDefaultFlecsThreadAllocationPolicyAsset::GetThreadCountAllocation(
	const TSolidNotNull<const UFlecsWorld*> InWorld) const
{
	return TTuple<EFlecsThreadAllocationType, uint32>(ThreadAllocationType, (int32)ThreadCount);
}
