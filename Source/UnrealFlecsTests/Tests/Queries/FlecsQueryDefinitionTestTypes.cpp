// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "FlecsQueryDefinitionTestTypes.h"

#include "Entities/FlecsEntityHandle.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"
#include "Worlds/FlecsWorldInterfaceObject.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsQueryDefinitionTestTypes)

namespace
{
	TArray<uint64> GCreatedGroupIds;
	TArray<uint64> GDestroyedGroupIds;
	TArray<uint64> GDestroyedGroupContextIds;
} // namespace

uint64 UE::Flecs::Tests::GroupByTableHasTag(
	const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld,
	FFlecsTableHandle InTable,
	FFlecsId InId,
	void* InContext)
{
	(void)InId;
	(void)InContext;

	const FFlecsEntityHandle TagComponent = InWorld->GetScriptStructEntity(FFlecsTestStruct_Tag::StaticStruct());
	return InTable.GetTable().has(TagComponent) ? GroupByWithTagGroupId : GroupByWithoutTagGroupId;
}

void UE::Flecs::Tests::ResetGroupByLifecycleLog()
{
	GCreatedGroupIds.Reset();
	GDestroyedGroupIds.Reset();
	GDestroyedGroupContextIds.Reset();
}

const TArray<uint64>& UE::Flecs::Tests::GetCreatedGroupIds()
{
	return GCreatedGroupIds;
}

const TArray<uint64>& UE::Flecs::Tests::GetDestroyedGroupIds()
{
	return GDestroyedGroupIds;
}

const TArray<uint64>& UE::Flecs::Tests::GetDestroyedGroupContextIds()
{
	return GDestroyedGroupContextIds;
}

void* UE::Flecs::Tests::RecordGroupCreated(
	const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld,
	uint64 InGroupId,
	void* InGroupByContext)
{
	(void)InWorld;
	(void)InGroupByContext;

	GCreatedGroupIds.Add(InGroupId);
	return new uint64(InGroupId);
}

void UE::Flecs::Tests::RecordGroupDestroyed(
	const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld,
	uint64 InGroupId,
	void* InGroupContext,
	void* InGroupByContext)
{
	(void)InWorld;
	(void)InGroupByContext;

	GDestroyedGroupIds.Add(InGroupId);
	uint64* GroupContext = static_cast<uint64*>(InGroupContext);
	GDestroyedGroupContextIds.Add(GroupContext != nullptr ? *GroupContext : 0);
	delete GroupContext;
}

UE::Flecs::Queries::FOrderByFunctionType FFlecsQueryOrderByCallbackDefinitionTest_ScriptStruct::GetOrderByFunction() const
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

UE::Flecs::Queries::FGroupByFunctionType FFlecsQueryGroupByCallbackDefinitionTest_TableHasTag::GetGroupByFunction() const
{
	return UE::Flecs::Tests::GroupByTableHasTag;
}

UE::Flecs::Queries::FOrderByFunctionType FFlecsQueryOrderByCallbackDefinitionTest_CPPType::GetOrderByFunction() const
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
