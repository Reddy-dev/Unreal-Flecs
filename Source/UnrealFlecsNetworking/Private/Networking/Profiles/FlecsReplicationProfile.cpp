// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Profiles/FlecsReplicationProfile.h"

#include "Properties/FlecsComponentProperties.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsReplicationProfile)

REGISTER_FLECS_COMPONENT(FFlecsReplicationProfileDefinition);
REGISTER_FLECS_COMPONENT(FFlecsReplicationProfileTag);

uint32 FFlecsReplicationProfileDefinition::AddParam(const TInstancedStruct<FFlecsReplicationProfileParamsBase>& InParam)
{
	solid_check(InParam.IsValid());
	return ParameterComponents.Add(InParam);
}

uint32 FFlecsReplicationProfileDefinition::AddParam(const TSolidNotNull<const UScriptStruct*> InParamStruct)
{
	return ParameterComponents.Add(TInstancedStruct<FFlecsReplicationProfileParamsBase>(InParamStruct));
}
