// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Systems/FlecsSystemHandle.h"

#include "Systems/FlecsSystemDefinition.h"
#include "Worlds/FlecsWorldInterfaceObject.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsSystemHandle)

FFlecsSystemHandle::FFlecsSystemHandle(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld,
	const FFlecsSystemDefinition& InSystemBuilder, const FString& InSystemName)
{
	flecs::system_builder<> Builder(InWorld->GetNativeFlecsWorld(), TCHAR_TO_UTF8(*InSystemName));
	InSystemBuilder.ApplyToSystem(InWorld, Builder);
	Entity = Builder.build();
	
	InSystemBuilder.PipelineInput.ApplyToSystemEntity(InWorld, *this);
}
