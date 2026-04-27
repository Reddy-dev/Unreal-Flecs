// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "General/FlecsThreadAllocationPolicyBaseAsset.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsThreadAllocationPolicyBaseAsset)

FPrimaryAssetId UFlecsThreadAllocationPolicyBaseAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("FlecsThreadAllocationPolicy"), GetFName());
}
