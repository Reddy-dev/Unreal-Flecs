// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Systems/FlecsSystemTickSourceInput.h"

#include "Systems/FlecsSystemObject.h"
#include "Worlds/FlecsWorldInterfaceObject.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsSystemTickSourceInput)

void FFlecsSystemTickSourceInput::ApplyToSystemDefinition(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InFlecsWorld,
	flecs::system_builder<>& InSystemBuilder) const
{
	if (InputType == EFlecsSystemTickSourceInput::None)
	{
		return;
	}
	else if (InputType == EFlecsSystemTickSourceInput::Type)
	{
		solid_checkf(TypeInput.IsValid(), TEXT("TickType input must be set when InputType is set to Type."));
		solid_checkf(!TypeInput.IsPair(), TEXT("TickType input cannot be a pair when InputType is set to Type."));\
	
		const FFlecsTermRefAtom_Internal TickSourceAtom = TypeInput.GetFirstTermRef(InFlecsWorld);
		solid_checkf(!TickSourceAtom.IsType<char*>(), TEXT("TickType input cannot be a string when InputType is set to Type."));
		
		const FFlecsId TickSourceId = TickSourceAtom.Get<FFlecsId>();
		solid_checkf(TickSourceId.IsValid(), 
			TEXT("TickType input does not reference a valid FlecsId component when InputType is set to Type."));
		
		InSystemBuilder.tick_source(TickSourceId);
	}
	else if (InputType == EFlecsSystemTickSourceInput::SystemClass)
	{
		solid_checkf(IsValid(SystemClassInput), TEXT("SystemClass input must be set when InputType is set to SystemClass."));
		
		solid_checkf(InFlecsWorld->IsFlecsObjectRegistered(SystemClassInput), 
			TEXT("SystemClass input %s is not registered in the FlecsWorld."), *SystemClassInput->GetName());
		
		const TSolidNotNull<const UFlecsSystemObject*> SystemClassRegisteredObject = 
			CastChecked<UFlecsSystemObject>(InFlecsWorld->GetRegisteredFlecsObject(SystemClassInput));
		
		const FFlecsEntityHandle SystemClassEntity = SystemClassRegisteredObject->GetEntityHandle();
		solid_checkf(SystemClassEntity.IsValid(), 
			TEXT("SystemClass input %s does not have a valid Flecs entity."), *SystemClassInput->GetName());
		
		InSystemBuilder.tick_source(SystemClassEntity);
	}
	else UNLIKELY_ATTRIBUTE
	{
		UNREACHABLE
	}
}
