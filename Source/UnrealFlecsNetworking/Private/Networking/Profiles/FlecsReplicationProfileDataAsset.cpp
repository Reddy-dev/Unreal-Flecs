// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Profiles/FlecsReplicationProfileDataAsset.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsReplicationProfileDataAsset)

FPrimaryAssetId UFlecsReplicationProfileDataAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId("FlecsReplicationProfileDataAsset", GetFName());
}
