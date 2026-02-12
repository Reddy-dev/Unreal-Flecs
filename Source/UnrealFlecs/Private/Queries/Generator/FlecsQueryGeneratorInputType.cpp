// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Queries/Generator/FlecsQueryGeneratorInputType.h"

#include "Worlds/FlecsWorld.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsQueryGeneratorInputType)

FFlecsId FFlecsQueryGeneratorInputType_ScriptStruct::GetFlecsIdOutput(
	const TSolidNotNull<const UFlecsWorld*> InWorld) const
{
	solid_checkf(ScriptStruct != nullptr, TEXT("FFlecsQueryGeneratorInputType_ScriptStruct::GetFlecsIdOutput: ScriptStruct is null!"));
	
	return InWorld->GetScriptStructEntity(ScriptStruct);
}

FFlecsId FFlecsQueryGeneratorInputType_ScriptEnum::GetFlecsIdOutput(
	const TSolidNotNull<const UFlecsWorld*> InWorld) const
{
	solid_checkf(ScriptEnum != nullptr, TEXT("FFlecsQueryGeneratorInputType_ScriptEnum::GetFlecsIdOutput: ScriptEnum is null!"));
	
	return InWorld->GetScriptEnumEntity(ScriptEnum);
}

FString FFlecsQueryGeneratorInputType_String::GetStringOutput() const
{
	return InputString;
}

FFlecsId FFlecsQueryGeneratorInputType_CPPType::GetFlecsIdOutput(const TSolidNotNull<const UFlecsWorld*> InWorld) const
{
	return InWorld->LookupEntityBySymbol_Internal(SymbolString);
}

FFlecsId FFlecsQueryGeneratorInputType_FlecsId::GetFlecsIdOutput(const TSolidNotNull<const UFlecsWorld*> InWorld) const
{
	return FlecsId;
}

FFlecsId FFlecsQueryGeneratorInputType_ScriptEnumConstant::GetFlecsIdOutput(
	const TSolidNotNull<const UFlecsWorld*> InWorld) const
{
	const TSolidNotNull<const UEnum*> EnumClass = EnumValue.Class;
	const FString EnumValueName = EnumClass->GetNameStringByValue(EnumValue.Value);
	
	const FFlecsEntityHandle EnumEntity = InWorld->GetScriptEnumEntity(EnumClass);
	const FFlecsEntityHandle EnumConstantEntity = EnumEntity.GetEnumConstant<FFlecsEntityHandle>(EnumValue);
	solid_checkf(EnumConstantEntity.IsValid(),
		TEXT("FFlecsQueryGeneratorInputType_ScriptEnumConstant::GetFlecsIdOutput: Could not find enum constant entity for value %s in enum %s!"),
		*EnumValueName, *EnumClass->GetName());
	
	return EnumConstantEntity;
}

void FFlecsQueryGeneratorInputType_Pair::CustomBuilderOutput(flecs::query_builder<>& Builder,
	const TSolidNotNull<const UFlecsWorld*> InWorld) const
{
	const EFlecsQueryGeneratorReturnType FirstReturnType = First.Get().ReturnType;
	const EFlecsQueryGeneratorReturnType SecondReturnType = Second.Get().ReturnType;
	
	solid_checkf(FirstReturnType != EFlecsQueryGeneratorReturnType::CustomBuilder,
		TEXT("FFlecsQueryGeneratorInputType_Pair::CustomBuilderOutput: First input type cannot be of CustomBuilder return type!"));
	solid_checkf(SecondReturnType != EFlecsQueryGeneratorReturnType::CustomBuilder,
		TEXT("FFlecsQueryGeneratorInputType_Pair::CustomBuilderOutput: Second input type cannot be of CustomBuilder return type!"));
	
	using FStringOrId = TVariant<FString, FFlecsId>;
	
	FStringOrId FirstOutput;
	FStringOrId SecondOutput;
	
	if (FirstReturnType == EFlecsQueryGeneratorReturnType::FlecsId)
	{
		FirstOutput = FStringOrId(TInPlaceType<FFlecsId>(), First.Get().GetFlecsIdOutput(InWorld));
	}
	else if (FirstReturnType == EFlecsQueryGeneratorReturnType::String)
	{
		FirstOutput = FStringOrId(TInPlaceType<FString>(), First.Get().GetStringOutput());
	}
	else if (FirstReturnType == EFlecsQueryGeneratorReturnType::CustomBuilder) UNLIKELY_ATTRIBUTE
	{
		solid_cassumef(false,
			TEXT("FFlecsQueryGeneratorInputType_Pair::CustomBuilderOutput: First input type cannot be of CustomBuilder return type!"));
	}
	else UNLIKELY_ATTRIBUTE
	{
		UNREACHABLE
	}
	
	if (SecondReturnType == EFlecsQueryGeneratorReturnType::FlecsId)
	{
		SecondOutput = FStringOrId(TInPlaceType<FFlecsId>(), Second.Get().GetFlecsIdOutput(InWorld));
	}
	else if (SecondReturnType == EFlecsQueryGeneratorReturnType::String)
	{
		SecondOutput = FStringOrId(TInPlaceType<FString>(), Second.Get().GetStringOutput());
	}
	else if (SecondReturnType == EFlecsQueryGeneratorReturnType::CustomBuilder) UNLIKELY_ATTRIBUTE
	{
		solid_checkf(false, TEXT("FFlecsQueryGeneratorInputType_Pair::CustomBuilderOutput: Second input type cannot be of CustomBuilder return type!"));
	}
	else UNLIKELY_ATTRIBUTE
	{
		UNREACHABLE
	}
	
	if (FirstOutput.IsType<FFlecsId>() && SecondOutput.IsType<FFlecsId>())
	{
		Builder.with(FirstOutput.Get<FFlecsId>(), SecondOutput.Get<FFlecsId>());
	}
	else if (FirstOutput.IsType<FFlecsId>() && SecondOutput.IsType<FString>())
	{
		const FString& SecondString = SecondOutput.Get<FString>();
		
		const char* CStr = TCHAR_TO_UTF8(*SecondString);
		solid_cassume(CStr != nullptr);
	
		const uint32 Length = static_cast<uint32>(FCStringAnsi::Strlen(CStr));
		
		const char* OwnedCStr = (const char*)FMemory::Malloc(Length + 1);
		FMemory::Memcpy((void*)OwnedCStr, (const void*)CStr, Length + 1);
		
		Builder.with(FirstOutput.Get<FFlecsId>(), OwnedCStr);
	}
	else if (FirstOutput.IsType<FString>() && SecondOutput.IsType<FFlecsId>())
	{
		const FString& FirstString = FirstOutput.Get<FString>();
		
		const char* CStr = TCHAR_TO_UTF8(*FirstString);
		solid_cassume(CStr != nullptr);
	
		const uint32 Length = static_cast<uint32>(FCStringAnsi::Strlen(CStr));
		
		const char* OwnedCStr = (const char*)FMemory::Malloc(Length + 1);
		FMemory::Memcpy((void*)OwnedCStr, (const void*)CStr, Length + 1);
		
		Builder.with(OwnedCStr, SecondOutput.Get<FFlecsId>());
	}
	else if (FirstOutput.IsType<FString>() && SecondOutput.IsType<FString>())
	{
		const FString& FirstString = FirstOutput.Get<FString>();
		const FString& SecondString = SecondOutput.Get<FString>();
		
		const char* CStrFirst = TCHAR_TO_UTF8(*FirstString);
		solid_cassume(CStrFirst != nullptr);
	
		const uint32 LengthFirst = static_cast<uint32>(FCStringAnsi::Strlen(CStrFirst));
		
		const char* OwnedCStrFirst = (const char*)FMemory::Malloc(LengthFirst + 1);
		FMemory::Memcpy((void*)OwnedCStrFirst, (const void*)CStrFirst, LengthFirst + 1);
		
		
		const char* CStrSecond = TCHAR_TO_UTF8(*SecondString);
		solid_cassume(CStrSecond != nullptr);
	
		const uint32 LengthSecond = static_cast<uint32>(FCStringAnsi::Strlen(CStrSecond));
		const char* OwnedCStrSecond = (const char*)FMemory::Malloc(LengthSecond + 1);
		FMemory::Memcpy((void*)OwnedCStrSecond, (const void*)CStrSecond, LengthSecond + 1);
		
		Builder.with(OwnedCStrFirst, OwnedCStrSecond);
	}
}

void FFlecsQueryGeneratorInputType_WithNameComponent::CustomBuilderOutput(flecs::query_builder<>& Builder,
	const TSolidNotNull<const UFlecsWorld*> InWorld) const
{
	Builder.with_name_component();
}
