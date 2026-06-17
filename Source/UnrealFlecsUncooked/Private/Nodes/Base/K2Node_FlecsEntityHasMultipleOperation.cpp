// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Nodes/Base/K2Node_FlecsEntityHasMultipleOperation.h"

#include "EdGraphSchema_K2.h"
#include "Framework/Commands/UIAction.h"
#include "K2Node_MakeArray.h"
#include "KismetCompiler.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "ScopedTransaction.h"
#include "ToolMenu.h"
#include "ToolMenuSection.h"

#include "Libraries/FlecsEntityHandleFunctionLibrary.h"
#include "SolidMacros/Macros.h"
#include "Types/SolidNotNull.h"

#include "Nodes/K2Node_FlecsCallFunction.h"
#include "Nodes/PinLogic/FlecsEntityViewResolver.h"
#include "Nodes/PinLogic/FlecsGenericInputPins.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_FlecsEntityHasMultipleOperation)

#define LOCTEXT_NAMESPACE "K2Node_FlecsEntityHasMultipleOperation"

void UK2Node_FlecsEntityHasMultipleOperation::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();

	GetResultPin()->PinFriendlyName = LOCTEXT("ResultPin", "Result");

	NumInputs = FMath::Max(NumInputs, MinimumInputCount);
	for (int32 InputIndex = 0; InputIndex < NumInputs; ++InputIndex)
	{
		AllocateInputPins(InputIndex);
	}
}

void UK2Node_FlecsEntityHasMultipleOperation::ExpandNode(
	FKismetCompilerContext& CompilerContext,
	UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	TArray<UK2Node_CallFunction*> ResolveFunctions;
	ResolveFunctions.Reserve(NumInputs);

	for (int32 InputIndex = 0; InputIndex < NumInputs; ++InputIndex)
	{
		const FFlecsGenericInputPins InputPins = GetInputPins(InputIndex);
		const TOptional<EFlecsBlueprintGenericPinTypes> SelectedType = InputPins.GetSelectedType(this);
		UEdGraphPin* ActiveValuePin = InputPins.GetActiveValuePin(this);
		if (!SelectedType.IsSet() || !ActiveValuePin)
		{
			Message_Error(TEXT("A Flecs component input type is invalid."));
			BreakAllNodeLinks();
			return;
		}

		const TOptional<UE::Flecs::EntityViewResolver::FResolverSpec> ResolverSpec =
			UE::Flecs::EntityViewResolver::GetResolverSpec(SelectedType.GetValue());
		if (!ResolverSpec.IsSet())
		{
			Message_Error(TEXT("A selected Flecs component input type is unsupported."));
			BreakAllNodeLinks();
			return;
		}

		UK2Node_CallFunction* ResolveFunction =
			UE::Flecs::EntityViewResolver::SpawnResolver(
				CompilerContext,
				this,
				SourceGraph,
				ResolverSpec.GetValue());

		CompilerContext.CopyPinLinksToIntermediate(
			*GetEntityPin(),
			*ResolveFunction->FindPinChecked(TEXT("Entity")));
		CompilerContext.MovePinLinksToIntermediate(
			*ActiveValuePin,
			*ResolveFunction->FindPinChecked(ResolverSpec.GetValue().ValueParameterName));

		ResolveFunctions.Add(ResolveFunction);
	}

	const TSolidNotNull<UK2Node_MakeArray*> MakeArray =
		CompilerContext.SpawnIntermediateNode<UK2Node_MakeArray>(this, SourceGraph);
	MakeArray->AllocateDefaultPins();
	for (int32 InputIndex = 1; InputIndex < NumInputs; ++InputIndex)
	{
		MakeArray->AddInputPin();
	}

	const TSolidNotNull<UK2Node_FlecsCallFunction*> HasFunction =
		CompilerContext.SpawnIntermediateNode<UK2Node_FlecsCallFunction>(this, SourceGraph);
	HasFunction->FunctionReference.SetExternalMember(
		GetHasFunctionName(),
		UFlecsEntityHandleFunctionLibrary::StaticClass());
	ConfigureIntermediateFunctionPurity(*HasFunction);
	HasFunction->AllocateDefaultPins();

	MoveExecLinksToIntermediate(CompilerContext, *HasFunction);
	CompilerContext.MovePinLinksToIntermediate(
		*GetEntityPin(),
		*HasFunction->FindPinChecked(TEXT("Entity")));
	CompilerContext.MovePinLinksToIntermediate(
		*GetResultPin(),
		*HasFunction->FindPinChecked(UEdGraphSchema_K2::PN_ReturnValue));

	const TSolidNotNull<const UEdGraphSchema_K2*> GraphSchema = GetDefault<UEdGraphSchema_K2>();
	bool bConnectedAllPins = GraphSchema->TryCreateConnection(
		MakeArray->GetOutputPin(),
		HasFunction->FindPinChecked(TEXT("ComponentIds")));

	for (int32 InputIndex = 0; InputIndex < NumInputs; ++InputIndex)
	{
		bConnectedAllPins &= GraphSchema->TryCreateConnection(
			ResolveFunctions[InputIndex]->FindPinChecked(UEdGraphSchema_K2::PN_ReturnValue),
			MakeArray->FindPinChecked(MakeArray->GetPinName(InputIndex)));
	}

	if (!bConnectedAllPins)
	{
		Message_Error(TEXT("Failed to connect the resolved Flecs ids to the multiple-component Has operation."));
	}

	BreakAllNodeLinks();
}

void UK2Node_FlecsEntityHasMultipleOperation::GetNodeContextMenuActions(
	UToolMenu* Menu,
	UGraphNodeContextMenuContext* Context) const
{
	Super::GetNodeContextMenuActions(Menu, Context);

	if (Context->bIsDebugging)
	{
		return;
	}

	FToolMenuSection& Section = Menu->AddSection(
		TEXT("FlecsHasMultiple"),
		LOCTEXT("FlecsHasMultipleHeader", "Flecs Inputs"));

	if (Context->Pin && CanRemovePin(Context->Pin))
	{
		Section.AddMenuEntry(
			TEXT("RemoveInput"),
			LOCTEXT("RemoveInput", "Remove Input"),
			LOCTEXT("RemoveInputTooltip", "Removes the final Flecs component input."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateUObject(
				const_cast<UK2Node_FlecsEntityHasMultipleOperation*>(this),
				&UK2Node_FlecsEntityHasMultipleOperation::RemoveInputPin,
				const_cast<UEdGraphPin*>(Context->Pin))));
	}
	else if (!Context->Pin && CanAddPin())
	{
		Section.AddMenuEntry(
			TEXT("AddInput"),
			LOCTEXT("AddInput", "Add Input"),
			LOCTEXT("AddInputTooltip", "Adds another Flecs component input."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateUObject(
				const_cast<UK2Node_FlecsEntityHasMultipleOperation*>(this),
				&UK2Node_FlecsEntityHasMultipleOperation::AddInputPin)));
	}
}

void UK2Node_FlecsEntityHasMultipleOperation::PinDefaultValueChanged(UEdGraphPin* Pin)
{
	Super::PinDefaultValueChanged(Pin);

	if (!Pin)
	{
		return;
	}

	const int32 InputIndex = FindInputIndex(Pin);
	if (InputIndex != INDEX_NONE && GetInputPins(InputIndex).IsSelectorPin(this, Pin))
	{
		GetInputPins(InputIndex).RefreshPins(this);

		if (UEdGraph* Graph = GetGraph())
		{
			Graph->NotifyGraphChanged();
		}
	}
}

void UK2Node_FlecsEntityHasMultipleOperation::PostReconstructNode()
{
	Super::PostReconstructNode();

	for (int32 InputIndex = 0; InputIndex < NumInputs; ++InputIndex)
	{
		GetInputPins(InputIndex).RefreshPins(this);
	}
}

void UK2Node_FlecsEntityHasMultipleOperation::AddInputPin()
{
	if (!CanAddPin())
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("AddInputTransaction", "Add Flecs Has Input"));
	Modify();

	AllocateInputPins(NumInputs);
	++NumInputs;

	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(GetBlueprint());
}

bool UK2Node_FlecsEntityHasMultipleOperation::CanAddPin() const
{
	return NumInputs < IK2Node_AddPinInterface::GetMaxInputPinsNum();
}

void UK2Node_FlecsEntityHasMultipleOperation::RemoveInputPin(UEdGraphPin* Pin)
{
	if (!CanRemovePin(Pin))
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("RemoveInputTransaction", "Remove Flecs Has Input"));
	Modify();

	const FFlecsGenericInputPins InputPins = GetInputPins(NumInputs - 1);
	TArray<UEdGraphPin*> PinsToRemove;
	for (UEdGraphPin* NodePin : Pins)
	{
		if (NodePin && InputPins.IsManagedPin(this, NodePin))
		{
			PinsToRemove.Add(NodePin);
		}
	}

	for (UEdGraphPin* PinToRemove : PinsToRemove)
	{
		RemovePin(PinToRemove);
	}

	--NumInputs;
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(GetBlueprint());
}

bool UK2Node_FlecsEntityHasMultipleOperation::CanRemovePin(const UEdGraphPin* Pin) const
{
	return Pin &&
		NumInputs > MinimumInputCount &&
		FindInputIndex(Pin) == NumInputs - 1;
}

FName UK2Node_FlecsEntityHasMultipleOperation::GetInputPrefix(const int32 InputIndex)
{
	return FName(*FString::Printf(TEXT("Component%d"), InputIndex));
}

FFlecsGenericInputPins UK2Node_FlecsEntityHasMultipleOperation::GetInputPins(const int32 InputIndex)
{
	return FFlecsGenericInputPins(GetInputPrefix(InputIndex));
}

void UK2Node_FlecsEntityHasMultipleOperation::AllocateInputPins(const int32 InputIndex)
{
	const FFlecsGenericInputPins InputPins = GetInputPins(InputIndex);
	InputPins.AllocatePins(this);

	const FText ComponentName = FText::Format(
		LOCTEXT("ComponentPinFormat", "Component {0}"),
		FText::AsNumber(InputIndex + 1));
	const FText ComponentTypeName = FText::Format(
		LOCTEXT("ComponentTypePinFormat", "Component {0} Type"),
		FText::AsNumber(InputIndex + 1));
	InputPins.SetFriendlyNames(this, ComponentTypeName, ComponentName);
}

int32 UK2Node_FlecsEntityHasMultipleOperation::FindInputIndex(const UEdGraphPin* Pin) const
{
	if (!Pin)
	{
		return INDEX_NONE;
	}

	for (int32 InputIndex = 0; InputIndex < NumInputs; ++InputIndex)
	{
		if (GetInputPins(InputIndex).IsManagedPin(this, Pin))
		{
			return InputIndex;
		}
	}

	return INDEX_NONE;
}

#undef LOCTEXT_NAMESPACE
