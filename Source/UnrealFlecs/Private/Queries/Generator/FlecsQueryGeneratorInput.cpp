// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Queries/Generator/FlecsQueryGeneratorInput.h"

#include "Queries/Generator/FlecsQueryGeneratorInputType.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsQueryGeneratorInput)

FFlecsTermRefAtom_Internal FFlecsQueryGeneratorInput::GetFirstTermRef(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld) const
{
	solid_checkf(First.IsValid(), TEXT("First term ref must be valid."));

	const FFlecsTermRef TermRef = First.Get().GetTermRefOutput(InWorld);
	
	return UE::Flecs::Queries::ToTermRefAtom(TermRef);
}

FFlecsTermRefAtom_Internal FFlecsQueryGeneratorInput::GetSecondTermRef(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld) const
{
	solid_checkf(Second.IsValid(), TEXT("Second term ref must be valid."));

	const FFlecsTermRef TermRef = Second.Get().GetTermRefOutput(InWorld);
	
	return UE::Flecs::Queries::ToTermRefAtom(TermRef);
}
