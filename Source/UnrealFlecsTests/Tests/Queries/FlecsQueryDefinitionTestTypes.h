// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Queries/Callbacks/FlecsGroupByCallbackDefinition.h"
#include "Queries/Callbacks/FlecsOrderByCallbackDefinition.h"

#include "FlecsQueryDefinitionTestTypes.generated.h"

namespace UE::Flecs::Tests
{
	inline constexpr uint64 GroupByWithoutTagGroupId = 1001;
	inline constexpr uint64 GroupByWithTagGroupId = 1002;

	UNREALFLECSTESTS_API uint64 GroupByTableHasTag(
		const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld,
		FFlecsTableHandle InTable,
		FFlecsId InId,
		void* InContext);

	UNREALFLECSTESTS_API void ResetGroupByLifecycleLog();
	UNREALFLECSTESTS_API const TArray<uint64>& GetCreatedGroupIds();
	UNREALFLECSTESTS_API const TArray<uint64>& GetDestroyedGroupIds();
	UNREALFLECSTESTS_API const TArray<uint64>& GetDestroyedGroupContextIds();

	UNREALFLECSTESTS_API void* RecordGroupCreated(
		const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld,
		uint64 InGroupId,
		void* InGroupByContext);

	UNREALFLECSTESTS_API void RecordGroupDestroyed(
		const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld,
		uint64 InGroupId,
		void* InGroupContext,
		void* InGroupByContext);
} // namespace UE::Flecs::Tests

UENUM()
enum class EFlecsTestQueryOrderByFunction : uint8
{
	Ascending,
	Descending,
}; // enum class ETestQueryOrderByFunction

USTRUCT(BlueprintInternalUseOnly)
struct UNREALFLECSTESTS_API FFlecsQueryOrderByCallbackDefinitionTest_ScriptStruct : public FFlecsOrderByCallbackDefinition
{
	GENERATED_BODY()
	
public:
	FORCEINLINE FFlecsQueryOrderByCallbackDefinitionTest_ScriptStruct(const EFlecsTestQueryOrderByFunction InOrderByFunction = EFlecsTestQueryOrderByFunction::Ascending)
		: OrderByFunction(InOrderByFunction)
	{
	}
	
	virtual NO_DISCARD UE::Flecs::Queries::FOrderByFunctionType GetOrderByFunction() const override;
	
	UPROPERTY()
	EFlecsTestQueryOrderByFunction OrderByFunction;
	
}; // struct FFlecsQueryOrderByCallbackDefinitionTest_ScriptStruct

USTRUCT(BlueprintInternalUseOnly)
struct UNREALFLECSTESTS_API FFlecsQueryOrderByCallbackDefinitionTest_CPPType : public FFlecsOrderByCallbackDefinition
{
	GENERATED_BODY()
	
public:
	FORCEINLINE FFlecsQueryOrderByCallbackDefinitionTest_CPPType(const EFlecsTestQueryOrderByFunction InOrderByFunction = EFlecsTestQueryOrderByFunction::Ascending)
		: OrderByFunction(InOrderByFunction)
	{
	}
	
	virtual NO_DISCARD UE::Flecs::Queries::FOrderByFunctionType GetOrderByFunction() const override;
	
	UPROPERTY()
	EFlecsTestQueryOrderByFunction OrderByFunction;
	
}; // struct FFlecsQueryOrderByCallbackDefinitionTest_ScriptStruct

USTRUCT(BlueprintInternalUseOnly)
struct UNREALFLECSTESTS_API FFlecsQueryGroupByCallbackDefinitionTest_TableHasTag : public FFlecsGroupByCallbackDefinition
{
	GENERATED_BODY()
	
public:
	
	virtual UE::Flecs::Queries::FGroupByFunctionType GetGroupByFunction() const override;
	
}; // struct FFlecsQueryGroupByCallbackDefinitionTest_TableHasTag
