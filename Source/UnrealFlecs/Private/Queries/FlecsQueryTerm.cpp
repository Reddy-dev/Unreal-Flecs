// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Queries/FlecsQueryTerm.h"

#include "Queries/Generator/FlecsQueryGeneratorInputType.h"
#include "Worlds/FlecsWorld.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsQueryTerm)

void FFlecsQueryTerm::ApplyToQueryBuilder(const TSolidNotNull<const UFlecsWorld*> InWorld,
	flecs::query_builder<>& InQueryBuilder) const
{
	const EFlecsQueryGeneratorReturnType ReturnType = InputType.Get().ReturnType;
	
	if (ReturnType == EFlecsQueryGeneratorReturnType::FlecsId)
	{
		const FFlecsId IdOutput = InputType.Get().GetFlecsIdOutput(InWorld);
		InQueryBuilder.with(IdOutput);
	}
	else if (ReturnType == EFlecsQueryGeneratorReturnType::String)
	{
		const FString StringOutput = InputType.Get().GetStringOutput();
	
		const char* CStr = TCHAR_TO_UTF8(*StringOutput);
		solid_cassume(CStr != nullptr);
	
		const uint32 Length = static_cast<uint32>(FCStringAnsi::Strlen(CStr));
		
		const char* OwnedCStr = (const char*)FMemory::Malloc(Length + 1);
		solid_cassume(OwnedCStr != nullptr);
		
		FMemory::Memcpy((void*)OwnedCStr, (const void*)CStr, Length + 1);
	
		InQueryBuilder.with(OwnedCStr);
	}
	else if (ReturnType == EFlecsQueryGeneratorReturnType::CustomBuilder)
	{
		InputType.Get().CustomBuilderOutput(InQueryBuilder, InWorld);
	}
	else UNLIKELY_ATTRIBUTE
	{
		UNREACHABLE
	}
}
