// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Queries/Callbacks/FlecsOrderByCallbackDefinition.h"

#include "FlecsQueryDefinitionTestTypes.generated.h"

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
	
	virtual NO_DISCARD Unreal::Flecs::Queries::FOrderByFunctionType GetOrderByFunction() const override;
	
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
	
	virtual NO_DISCARD Unreal::Flecs::Queries::FOrderByFunctionType GetOrderByFunction() const override;
	
	UPROPERTY()
	EFlecsTestQueryOrderByFunction OrderByFunction;
	
}; // struct FFlecsQueryOrderByCallbackDefinitionTest_ScriptStruct