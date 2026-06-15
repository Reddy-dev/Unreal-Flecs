// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Nodes/K2Node_FlecsEntityRemoveOperation.h"

#include "EdGraphSchema_K2.h"
#include "K2Node_CallFunction.h"
#include "KismetCompiler.h"

#include "Libraries/FlecsEntityHandleFunctionLibrary.h"
#include "Types/SolidNotNull.h"

#include "Nodes/FlecsGenericNodeTypes.h"
#include "Nodes/PinLogic/FlecsGenericInputPins.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_FlecsEntityRemoveOperation)

namespace UE::Flecs::RemoveNode
{
	struct FOperationSpec
	{
		FName FunctionName;
		FName ValueParameterName;
	};

	TOptional<FOperationSpec> GetOperationSpec(const EFlecsBlueprintGenericPinTypes Type)
	{
		switch (Type)
		{
		case EFlecsBlueprintGenericPinTypes::Id:
			return FOperationSpec{
				GET_FUNCTION_NAME_CHECKED(
					UFlecsEntityHandleFunctionLibrary,
					EntityHandle_RemoveId),
				TEXT("ComponentId")
			};
		case EFlecsBlueprintGenericPinTypes::ScriptStruct:
			return FOperationSpec{
				GET_FUNCTION_NAME_CHECKED(
					UFlecsEntityHandleFunctionLibrary,
					EntityHandle_RemoveScriptStruct),
				TEXT("ScriptStruct")
			};
		case EFlecsBlueprintGenericPinTypes::Enum:
			return FOperationSpec{
				GET_FUNCTION_NAME_CHECKED(
					UFlecsEntityHandleFunctionLibrary,
					EntityHandle_RemoveEnum),
				TEXT("Enum")
			};
		case EFlecsBlueprintGenericPinTypes::EnumConstant:
			return FOperationSpec{
				GET_FUNCTION_NAME_CHECKED(
					UFlecsEntityHandleFunctionLibrary,
					EntityHandle_RemoveEnumConstant),
				TEXT("EnumConstant")
			};
		case EFlecsBlueprintGenericPinTypes::GameplayTag:
			return FOperationSpec{
				GET_FUNCTION_NAME_CHECKED(
					UFlecsEntityHandleFunctionLibrary,
					EntityHandle_RemoveGameplayTag),
				TEXT("GameplayTag")
			};
		default:
			return {};
		}
	}
} // namespace UE::Flecs::RemoveNode

void UK2Node_FlecsEntityRemoveOperation::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();
	GetComponentPins().AllocatePins(this);
	GetComponentPins().SetFriendlyNames(
		this,
		FText::FromString(TEXT("Component Type")),
		FText::FromString(TEXT("Component")));
}

void UK2Node_FlecsEntityRemoveOperation::ExpandNode(
	FKismetCompilerContext& CompilerContext,
	UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	const TOptional<EFlecsBlueprintGenericPinTypes> SelectedType =
		GetComponentPins().GetSelectedType(this);
	UEdGraphPin* ActiveValuePin = GetComponentPins().GetActiveValuePin(this);
	const TOptional<UE::Flecs::RemoveNode::FOperationSpec> OperationSpec =
		SelectedType.IsSet()
			? UE::Flecs::RemoveNode::GetOperationSpec(SelectedType.GetValue())
			: TOptional<UE::Flecs::RemoveNode::FOperationSpec>();
	if (!ActiveValuePin || !OperationSpec.IsSet())
	{
		Message_Error(TEXT("The Flecs component input type is invalid."));
		BreakAllNodeLinks();
		return;
	}

	const TSolidNotNull<UK2Node_CallFunction*> RemoveFunction =
		CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	RemoveFunction->FunctionReference.SetExternalMember(
		OperationSpec.GetValue().FunctionName,
		UFlecsEntityHandleFunctionLibrary::StaticClass());
	RemoveFunction->AllocateDefaultPins();

	CompilerContext.MovePinLinksToIntermediate(*GetExecPin(), *RemoveFunction->GetExecPin());
	CompilerContext.MovePinLinksToIntermediate(*GetThenPin(), *RemoveFunction->GetThenPin());
	CompilerContext.MovePinLinksToIntermediate(
		*GetEntityPin(),
		*RemoveFunction->FindPinChecked(TEXT("Entity")));
	CompilerContext.MovePinLinksToIntermediate(
		*ActiveValuePin,
		*RemoveFunction->FindPinChecked(OperationSpec.GetValue().ValueParameterName));

	BreakAllNodeLinks();
}

FText UK2Node_FlecsEntityRemoveOperation::GetNodeTitle(
	ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Remove Component from Flecs Entity"));
}

void UK2Node_FlecsEntityRemoveOperation::PinDefaultValueChanged(UEdGraphPin* Pin)
{
	Super::PinDefaultValueChanged(Pin);

	if (Pin && GetComponentPins().IsSelectorPin(this, Pin))
	{
		GetComponentPins().RefreshPins(this);

		if (UEdGraph* Graph = GetGraph())
		{
			Graph->NotifyGraphChanged();
		}
	}
}

void UK2Node_FlecsEntityRemoveOperation::PostReconstructNode()
{
	Super::PostReconstructNode();
	GetComponentPins().RefreshPins(this);
}

const FFlecsGenericInputPins& UK2Node_FlecsEntityRemoveOperation::GetComponentPins()
{
	static const FFlecsGenericInputPins ComponentPins;
	return ComponentPins;
}
