// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Systems/FlecsSystemPhaseInput.h"

#include "Worlds/FlecsWorld.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsSystemPhaseInput)

void FFlecsSystemPhaseInput::ApplyToSystemDefinition(const TSolidNotNull<const UFlecsWorld*> InFlecsWorld,
	flecs::system_builder<>& InSystemBuilder) const
{
	if (Type == EFlecsSystemPhaseInputType::None)
	{
		return;
	} 
	else if (Type == EFlecsSystemPhaseInputType::FlecsPhase)
	{
		const FFlecsId PhaseId = ConvertPhaseTypeToId(FlecsPhase);
		solid_checkf(PhaseId.IsValid(), TEXT("Invalid phase identifier for system phase input of type 'FlecsPhase'."));
		
		InSystemBuilder.kind(PhaseId);
	}
	else if (Type == EFlecsSystemPhaseInputType::Type)
	{
		solid_checkf(PhaseInput.IsValid(), TEXT("Invalid phase input for system phase input of type 'Type'."));
		
		const FFlecsTermRefAtom_Internal FirstAtom = PhaseInput.GetFirstTermRef(InFlecsWorld);
		
		solid_checkf(!FirstAtom.IsType<char*>(), TEXT("String identifiers are not supported for system phase input of type 'Type'."));
		
		FFlecsTermRefAtom_Internal SecondAtom;
		if (PhaseInput.IsPair())
		{
			SecondAtom = PhaseInput.GetSecondTermRef(InFlecsWorld);
			solid_checkf(!SecondAtom.IsType<char*>(), TEXT("String identifiers are not supported for system phase input of type 'Type'."));
		}
		
		FFlecsId OutputId;
		
		const FFlecsId FirstId = FirstAtom.Get<FFlecsId>();
		solid_checkf(FirstId.IsValid(), TEXT("Invalid first identifier in phase input for system phase input of type 'Type'."));
		
		if (PhaseInput.IsPair())
		{
			const FFlecsId SecondId = SecondAtom.Get<FFlecsId>();
			solid_checkf(SecondId.IsValid(), TEXT("Invalid second identifier in phase input for system phase input of type 'Type'."));
			
			OutputId = FFlecsId::MakePair(FirstId, SecondId);
		}
		else
		{
			OutputId = FirstId;
		}
		
		InSystemBuilder.kind(OutputId);
	}
	else UNLIKELY_ATTRIBUTE
	{
		UNREACHABLE
	}
}
