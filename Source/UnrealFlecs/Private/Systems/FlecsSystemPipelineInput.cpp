// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Systems/FlecsSystemPipelineInput.h"

#include "Systems/FlecsSystemHandle.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsSystemPipelineInput)

void FFlecsSystemPipelineInput::ApplyToSystemEntity(const TSolidNotNull<const UFlecsWorld*> InWorld,
	const FFlecsSystemHandle& InSystem) const
{
	solid_checkf(InSystem.IsValid(), TEXT("Invalid system handle provided when applying pipeline input to system entity."));
	
	if (InputType == EFlecsSystemPipelineInputType::None)
	{
		return;
	}
	else if (InputType == EFlecsSystemPipelineInputType::Tag)
	{
		solid_checkf(Tag.IsValid(), TEXT("Invalid gameplay tag provided when applying pipeline input to system entity."));
		InSystem.Add(Tag);
	}
	else if (InputType == EFlecsSystemPipelineInputType::Type)
	{
		const FFlecsTermRefAtom_Internal FirstAtom = TypeInput.GetFirstTermRef(InWorld);
		solid_checkf(!FirstAtom.IsType<char*>(),
			TEXT("Invalid type provided when applying pipeline input to system entity, expected a component or tag type but got a string."));
		
		FFlecsTermRefAtom_Internal SecondAtom;
		if (TypeInput.IsPair())
		{
			SecondAtom = TypeInput.GetSecondTermRef(InWorld);
			solid_checkf(!SecondAtom.IsType<char*>(),
				TEXT("Invalid type provided when applying pipeline input to system entity, expected a component or tag type but got a string."));
		}
		
		FFlecsId OutputId;
		
		const FFlecsId FirstId = FirstAtom.Get<FFlecsId>();

		if (TypeInput.IsPair())
		{
			const FFlecsId SecondId = SecondAtom.Get<FFlecsId>();
			solid_checkf(SecondId.IsValid(),
				TEXT("Invalid second type provided when applying pipeline input to system entity, expected a valid component or tag type."));
			
			OutputId = FFlecsId::MakePair(FirstId, SecondId);
		}
		else
		{
			OutputId = FirstId;
		}
		
		InSystem.Add(OutputId);
	}
}
