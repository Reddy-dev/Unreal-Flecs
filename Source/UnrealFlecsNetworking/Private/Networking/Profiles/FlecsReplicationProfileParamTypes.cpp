// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Profiles/FlecsReplicationProfileParamTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsReplicationProfileParamTypes)

REGISTER_FLECS_COMPONENT(FFlecsReplicationProfileCullDistance);
REGISTER_FLECS_COMPONENT(FFlecsReplicationProfileUpdateRate);

void FFlecsReplicationProfileCullDistance::ApplyToEntity(const FFlecsEntityHandle& InEntity) const
{
	InEntity.Set<FFlecsReplicationProfileCullDistance>(*this);
}

void FFlecsReplicationProfileUpdateRate::ApplyToEntity(const FFlecsEntityHandle& InEntity) const
{
	InEntity.Set<FFlecsReplicationProfileUpdateRate>(*this);
}

