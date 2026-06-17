// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Nodes/K2Node_FlecsEntityAddOperation.h"

#include "K2Node_CallFunction.h"
#include "KismetCompiler.h"

#include "Libraries/FlecsEntityHandleFunctionLibrary.h"
#include "Types/SolidNotNull.h"

#include "Nodes/FlecsGenericNodeTypes.h"
#include "Nodes/PinLogic/FlecsGenericInputPins.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_FlecsEntityAddOperation)

namespace UE::Flecs::AddNode
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
					EntityHandle_AddId),
				TEXT("ComponentId")
			};
		case EFlecsBlueprintGenericPinTypes::ScriptStruct:
			return FOperationSpec{
				GET_FUNCTION_NAME_CHECKED(
					UFlecsEntityHandleFunctionLibrary,
					EntityHandle_AddScriptStruct),
				TEXT("ScriptStruct")
			};
		case EFlecsBlueprintGenericPinTypes::Enum:
			return FOperationSpec{
				GET_FUNCTION_NAME_CHECKED(
					UFlecsEntityHandleFunctionLibrary,
					EntityHandle_AddEnum),
				TEXT("Enum")
			};
		case EFlecsBlueprintGenericPinTypes::EnumConstant:
			return FOperationSpec{
				GET_FUNCTION_NAME_CHECKED(
					UFlecsEntityHandleFunctionLibrary,
					EntityHandle_AddEnumConstant),
				TEXT("EnumConstant")
			};
		case EFlecsBlueprintGenericPinTypes::GameplayTag:
			return FOperationSpec{
				GET_FUNCTION_NAME_CHECKED(
					UFlecsEntityHandleFunctionLibrary,
					EntityHandle_AddGameplayTag),
				TEXT("GameplayTag")
			};
		default:
			return {};
		}
	}
} // namespace UE::Flecs::AddNode

void UK2Node_FlecsEntityAddOperation::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();
	GetComponentPins().AllocatePins(this);
}

void UK2Node_FlecsEntityAddOperation::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	const TOptional<EFlecsBlueprintGenericPinTypes> SelectedType = GetComponentPins().GetSelectedType(this);
	UEdGraphPin* ActiveValuePin = GetComponentPins().GetActiveValuePin(this);
	const TOptional<UE::Flecs::AddNode::FOperationSpec> OperationSpec =
		SelectedType.IsSet()
			? UE::Flecs::AddNode::GetOperationSpec(SelectedType.GetValue())
			: TOptional<UE::Flecs::AddNode::FOperationSpec>();
	if (!ActiveValuePin || !OperationSpec.IsSet())
	{
		Message_Error(TEXT("The Flecs component input type is invalid."));
		BreakAllNodeLinks();
		return;
	}

	const TSolidNotNull<UK2Node_CallFunction*> AddFunction =
		CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	AddFunction->FunctionReference.SetExternalMember(
		OperationSpec.GetValue().FunctionName,
		UFlecsEntityHandleFunctionLibrary::StaticClass());
	AddFunction->AllocateDefaultPins();

	CompilerContext.MovePinLinksToIntermediate(*GetExecPin(), *AddFunction->GetExecPin());
	CompilerContext.MovePinLinksToIntermediate(*GetThenPin(), *AddFunction->GetThenPin());
	CompilerContext.MovePinLinksToIntermediate(
		*GetEntityPin(),
		*AddFunction->FindPinChecked(TEXT("Entity")));
	CompilerContext.MovePinLinksToIntermediate(
		*ActiveValuePin,
		*AddFunction->FindPinChecked(OperationSpec.GetValue().ValueParameterName));

	BreakAllNodeLinks();
}

FText UK2Node_FlecsEntityAddOperation::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Add Component to Flecs Entity"));
}

void UK2Node_FlecsEntityAddOperation::ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins)
{
	Super::ReallocatePinsDuringReconstruction(OldPins);
}

void UK2Node_FlecsEntityAddOperation::PinDefaultValueChanged(UEdGraphPin* Pin)
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

void UK2Node_FlecsEntityAddOperation::PostReconstructNode()
{
	Super::PostReconstructNode();
	GetComponentPins().RefreshPins(this);
}

const FFlecsGenericInputPins& UK2Node_FlecsEntityAddOperation::GetComponentPins()
{
	static const FFlecsGenericInputPins ComponentPins;
	return ComponentPins;
}
