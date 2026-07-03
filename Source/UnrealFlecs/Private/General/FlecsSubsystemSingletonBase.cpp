// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "General/FlecsSubsystemSingletonBase.h"

#include "Worlds/FlecsAbstractWorldSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsSubsystemSingletonBase)

REGISTER_FLECS_COMPONENT(FFlecsSubsystemSingletonBase);

TSolidNotNull<UFlecsAbstractWorldSubsystem*> FFlecsSubsystemSingletonBase::GetSubsystemChecked() const
{
	solid_cassume(Subsystem != nullptr);
	solid_check(::IsValid(Subsystem));
	return Subsystem;
}
