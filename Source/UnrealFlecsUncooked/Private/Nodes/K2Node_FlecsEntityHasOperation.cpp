// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Nodes/K2Node_FlecsEntityHasOperation.h"

#include "EdGraphSchema_K2.h"
#include "KismetCompiler.h"

#include "Libraries/FlecsEntityHandleFunctionLibrary.h"
#include "Types/SolidNotNull.h"

#include "Nodes/K2Node_FlecsCallFunction.h"
#include "Nodes/PinLogic/FlecsGenericInputPins.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_FlecsEntityHasOperation)

namespace UE::Flecs::HasNode
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
					EntityView_HasId),
				TEXT("ComponentId")
			};
		case EFlecsBlueprintGenericPinTypes::ScriptStruct:
			return FOperationSpec{
				GET_FUNCTION_NAME_CHECKED(
					UFlecsEntityHandleFunctionLibrary,
					EntityView_HasScriptStruct),
				TEXT("ScriptStruct")
			};
		case EFlecsBlueprintGenericPinTypes::Enum:
			return FOperationSpec{
				GET_FUNCTION_NAME_CHECKED(
					UFlecsEntityHandleFunctionLibrary,
					EntityView_HasEnum),
				TEXT("Enum")
			};
		case EFlecsBlueprintGenericPinTypes::EnumConstant:
			return FOperationSpec{
				GET_FUNCTION_NAME_CHECKED(
					UFlecsEntityHandleFunctionLibrary,
					EntityView_HasEnumConstant),
				TEXT("EnumConstant")
			};
		case EFlecsBlueprintGenericPinTypes::GameplayTag:
			return FOperationSpec{
				GET_FUNCTION_NAME_CHECKED(
					UFlecsEntityHandleFunctionLibrary,
					EntityView_HasGameplayTag),
				TEXT("GameplayTag")
			};
		default:
			return {};
		}
	}
} // namespace UE::Flecs::HasNode

void UK2Node_FlecsEntityHasOperation::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();
	GetComponentPins().AllocatePins(this);
}

void UK2Node_FlecsEntityHasOperation::ExpandNode(
	FKismetCompilerContext& CompilerContext,
	UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	const TOptional<EFlecsBlueprintGenericPinTypes> SelectedType = GetComponentPins().GetSelectedType(this);
	UEdGraphPin* ActiveValuePin = GetComponentPins().GetActiveValuePin(this);
	const TOptional<UE::Flecs::HasNode::FOperationSpec> OperationSpec =
		SelectedType.IsSet()
			? UE::Flecs::HasNode::GetOperationSpec(SelectedType.GetValue())
			: TOptional<UE::Flecs::HasNode::FOperationSpec>();
	if (!ActiveValuePin || !OperationSpec.IsSet())
	{
		Message_Error(TEXT("The Flecs component input type is invalid."));
		BreakAllNodeLinks();
		return;
	}

	const TSolidNotNull<UK2Node_FlecsCallFunction*> HasFunction =
		CompilerContext.SpawnIntermediateNode<UK2Node_FlecsCallFunction>(this, SourceGraph);
	HasFunction->FunctionReference.SetExternalMember(
		OperationSpec.GetValue().FunctionName,
		UFlecsEntityHandleFunctionLibrary::StaticClass());
	ConfigureIntermediateFunctionPurity(*HasFunction);
	HasFunction->AllocateDefaultPins();

	MoveExecLinksToIntermediate(CompilerContext, *HasFunction);
	CompilerContext.MovePinLinksToIntermediate(
		*GetEntityPin(),
		*HasFunction->FindPinChecked(TEXT("Entity")));
	CompilerContext.MovePinLinksToIntermediate(
		*ActiveValuePin,
		*HasFunction->FindPinChecked(OperationSpec.GetValue().ValueParameterName));
	CompilerContext.MovePinLinksToIntermediate(
		*GetResultPin(),
		*HasFunction->FindPinChecked(UEdGraphSchema_K2::PN_ReturnValue));

	BreakAllNodeLinks();
}

FText UK2Node_FlecsEntityHasOperation::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Has Component on Flecs Entity"));
}

void UK2Node_FlecsEntityHasOperation::PinDefaultValueChanged(UEdGraphPin* Pin)
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

void UK2Node_FlecsEntityHasOperation::PostReconstructNode()
{
	Super::PostReconstructNode();
	GetComponentPins().RefreshPins(this);
}

const FFlecsGenericInputPins& UK2Node_FlecsEntityHasOperation::GetComponentPins()
{
	static const FFlecsGenericInputPins ComponentPins;
	return ComponentPins;
}
