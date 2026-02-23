// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Queries/Generator/FlecsQueryGeneratorInputType.h"

#include "Queries/FlecsQueryBuilderView.h"
#include "Worlds/FlecsWorld.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsQueryGeneratorInputType)

FFlecsTermRef FFlecsQueryGeneratorInputType::GetTermRefOutput(const TSolidNotNull<const UFlecsWorld*> InWorld) const
{
	if (ReturnType == EFlecsQueryGeneratorReturnType::FlecsId)
	{
		return FFlecsTermRef(TInPlaceType<FFlecsId>(), GetFlecsIdOutput(InWorld));
	}
	else if (ReturnType == EFlecsQueryGeneratorReturnType::String)
	{
		return FFlecsTermRef(TInPlaceType<FString>(), GetStringOutput());
	}
	else UNLIKELY_ATTRIBUTE
	{
		solid_checkf(false, TEXT("Invalid return type for query generator input type!"));
		return FFlecsTermRef();
	}
}

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
