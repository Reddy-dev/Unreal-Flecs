// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "K2Node_CallFunction.h"
#include "KismetCompiler.h"

#include "Libraries/FlecsEntityHandleFunctionLibrary.h"

#include "Nodes/FlecsGenericNodeTypes.h"

namespace UE::Flecs::EntityViewResolver
{
	struct FResolverSpec
	{
		FName FunctionName;
		FName ValueParameterName;
		bool bCompositePair = false;
	};

	inline TOptional<FResolverSpec> GetResolverSpec(const EFlecsBlueprintGenericPinTypes Type)
	{
		switch (Type)
		{
		case EFlecsBlueprintGenericPinTypes::Id:
			return FResolverSpec{
				GET_FUNCTION_NAME_CHECKED(UFlecsEntityHandleFunctionLibrary, EntityView_ResolveId),
				TEXT("Id")
			};
		case EFlecsBlueprintGenericPinTypes::ScriptStruct:
			return FResolverSpec{
				GET_FUNCTION_NAME_CHECKED(UFlecsEntityHandleFunctionLibrary, EntityView_ResolveScriptStructId),
				TEXT("ScriptStruct")
			};
		case EFlecsBlueprintGenericPinTypes::Enum:
			return FResolverSpec{
				GET_FUNCTION_NAME_CHECKED(UFlecsEntityHandleFunctionLibrary, EntityView_ResolveEnumId),
				TEXT("Enum")
			};
		case EFlecsBlueprintGenericPinTypes::EnumConstant:
			return FResolverSpec{
				GET_FUNCTION_NAME_CHECKED(UFlecsEntityHandleFunctionLibrary, EntityView_ResolveEnumConstantId),
				TEXT("EnumConstant")
			};
		case EFlecsBlueprintGenericPinTypes::GameplayTag:
			return FResolverSpec{
				GET_FUNCTION_NAME_CHECKED(UFlecsEntityHandleFunctionLibrary, EntityView_ResolveGameplayTagId),
				TEXT("GameplayTag")
			};
		default:
			return {};
		}
	}

	inline UK2Node_CallFunction* SpawnResolver(
		FKismetCompilerContext& CompilerContext,
		UK2Node* SourceNode,
		UEdGraph* SourceGraph,
		const FResolverSpec& ResolverSpec)
	{
		UK2Node_CallFunction* ResolveFunction =
			CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(SourceNode, SourceGraph);
		ResolveFunction->FunctionReference.SetExternalMember(
			ResolverSpec.FunctionName,
			UFlecsEntityHandleFunctionLibrary::StaticClass());
		ResolveFunction->AllocateDefaultPins();
		return ResolveFunction;
	}
} // namespace UE::Flecs::EntityViewResolver
