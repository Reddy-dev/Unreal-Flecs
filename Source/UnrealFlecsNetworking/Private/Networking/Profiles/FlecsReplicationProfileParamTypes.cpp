// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Profiles/FlecsReplicationProfileParamTypes.h"

#include "Networking/Profiles/FlecsNetAlwaysRelevantTag.h"
#include "Networking/Profiles/FlecsReplicationCullDistanceComponent.h"
#include "Networking/Profiles/FlecsReplicationUpdateRateComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsReplicationProfileParamTypes)

REGISTER_FLECS_COMPONENT(FFlecsReplicationProfileCullDistance);
REGISTER_FLECS_COMPONENT(FFlecsReplicationProfileUpdateRate);
REGISTER_FLECS_COMPONENT(FFlecsReplicationProfileAlwaysRelevant);

void FFlecsReplicationProfileCullDistance::ApplyToEntity(const FFlecsEntityHandle& InEntity) const
{
	InEntity.Set<FFlecsReplicationCullDistanceComponent>({.CullDistance=CullDistance});
}

void FFlecsReplicationProfileUpdateRate::ApplyToEntity(const FFlecsEntityHandle& InEntity) const
{
	InEntity.Set<FFlecsReplicationUpdateRateComponent>({.UpdateRate=UpdateRate});
}

void FFlecsReplicationProfileAlwaysRelevant::ApplyToEntity(const FFlecsEntityHandle& InEntity) const
{
	InEntity.Add<FFlecsNetAlwaysRelevantTag>();
}

