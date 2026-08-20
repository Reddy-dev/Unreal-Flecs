// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Profiles/FlecsReplicationProfileParamTypes.h"

#include "Networking/Profiles/FlecsNetAlwaysRelevantTag.h"
#include "Networking/Profiles/FlecsProfileRelationshipTypes.h"
#include "Networking/Profiles/FlecsReplicationCullDistanceComponent.h"
#include "Networking/Profiles/FlecsReplicationUpdateRateComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsReplicationProfileParamTypes)

REGISTER_FLECS_COMPONENT(FFlecsReplicationProfileCullDistance);
REGISTER_FLECS_COMPONENT(FFlecsReplicationProfileUpdateRate);
REGISTER_FLECS_COMPONENT(FFlecsReplicationProfileAlwaysRelevant);

REGISTER_FLECS_COMPONENT(FFlecsReplicationProfileObjectPrioritizer);
REGISTER_FLECS_COMPONENT(FFlecsReplicationProfileNetFilter);
REGISTER_FLECS_COMPONENT(FFlecsReplicationProfileNetShardSelector);

namespace
{
	template <typename TRelationship>
	void ApplyProfileNameRelationship(const FFlecsEntityHandle& InEntity, const FName& InName)
	{
		if (InName.IsNone())
		{
			return;
		}

		FFlecsNetProfileNameTarget Target;
		Target.Name = InName;
		InEntity.SetPair<TRelationship, FFlecsNetProfileNameTarget>(MoveTemp(Target));
	}
	
} // namespace

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

void FFlecsReplicationProfileObjectPrioritizer::ApplyToEntity(const FFlecsEntityHandle& InEntity) const
{
	ApplyProfileNameRelationship<FFlecsObjectPrioritizerRelationship>(InEntity, Name);
}

void FFlecsReplicationProfileNetFilter::ApplyToEntity(const FFlecsEntityHandle& InEntity) const
{
	ApplyProfileNameRelationship<FFlecsNetFilterRelationship>(InEntity, Name);
}

void FFlecsReplicationProfileNetShardSelector::ApplyToEntity(const FFlecsEntityHandle& InEntity) const
{
	ApplyProfileNameRelationship<FFlecsNetShardSelectorRelationship>(InEntity, Name);
}

