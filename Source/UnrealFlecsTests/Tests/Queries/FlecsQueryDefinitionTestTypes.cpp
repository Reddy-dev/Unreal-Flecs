// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "FlecsQueryDefinitionTestTypes.h"

#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsQueryDefinitionTestTypes)

Unreal::Flecs::Queries::FOrderByFunctionType FFlecsQueryOrderByCallbackDefinitionTest_ScriptStruct::GetOrderByFunction() const
{
	if (OrderByFunction == EFlecsTestQueryOrderByFunction::Ascending)
	{
		return [](FFlecsId, const void* A, FFlecsId, const void* B) -> int32
		{
			const FFlecsTestStruct_Value* StructA = static_cast<const FFlecsTestStruct_Value*>(A);
			const FFlecsTestStruct_Value* StructB = static_cast<const FFlecsTestStruct_Value*>(B);
			return StructA->Value - StructB->Value;
		};
	}
	else // Descending
	{
		return [](FFlecsId, const void* A, FFlecsId, const void* B) -> int32
		{
			const FFlecsTestStruct_Value* StructA = static_cast<const FFlecsTestStruct_Value*>(A);
			const FFlecsTestStruct_Value* StructB = static_cast<const FFlecsTestStruct_Value*>(B);
			return StructB->Value - StructA->Value;
		};
	}
}

Unreal::Flecs::Queries::FOrderByFunctionType FFlecsQueryOrderByCallbackDefinitionTest_CPPType::GetOrderByFunction() const
{
	if (OrderByFunction == EFlecsTestQueryOrderByFunction::Ascending)
	{
		return [](FFlecsId, const void* A, FFlecsId, const void* B) -> int32
		{
			const FFlecsTest_CPPStructValue* StructA = static_cast<const FFlecsTest_CPPStructValue*>(A);
			const FFlecsTest_CPPStructValue* StructB = static_cast<const FFlecsTest_CPPStructValue*>(B);
			return StructA->Value - StructB->Value;
		};
	}
	else // Descending
	{
		return [](FFlecsId, const void* A, FFlecsId, const void* B) -> int32
		{
			const FFlecsTest_CPPStructValue* StructA = static_cast<const FFlecsTest_CPPStructValue*>(A);
			const FFlecsTest_CPPStructValue* StructB = static_cast<const FFlecsTest_CPPStructValue*>(B);
			return StructB->Value - StructA->Value;
		};
	}
}


