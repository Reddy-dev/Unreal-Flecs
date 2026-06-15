// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Nodes/Base/K2Node_FlecsGenericEntityCheck.h"

#include "EdGraphSchema_K2.h"
#include "Framework/Commands/UIAction.h"
#include "K2Node_CallFunction.h"
#include "KismetCompiler.h"
#include "ScopedTransaction.h"
#include "ToolMenu.h"
#include "ToolMenuSection.h"

#include "Entities/FlecsEntityView.h"
#include "SolidMacros/Macros.h"

#include "Nodes/K2Node_FlecsCallFunction.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_FlecsGenericEntityCheck)

#define LOCTEXT_NAMESPACE "K2Node_FlecsGenericEntityCheck"

void UK2Node_FlecsGenericEntityCheck::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();

	if (!IsNodePure())
	{
		CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);
		CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);
	}

	UEdGraphNode::FCreatePinParams EntityPinParams;
	EntityPinParams.bIsConst = true;
	EntityPinParams.bIsReference = true;

	CreatePin(
		EGPD_Input,
		UEdGraphSchema_K2::PC_Struct,
		FFlecsEntityView::StaticStruct(),
		TEXT("Entity"),
		EntityPinParams);
	CreatePin(
		EGPD_Output,
		UEdGraphSchema_K2::PC_Boolean,
		UEdGraphSchema_K2::PN_ReturnValue);
}

void UK2Node_FlecsGenericEntityCheck::GetNodeContextMenuActions(
	UToolMenu* Menu,
	UGraphNodeContextMenuContext* Context) const
{
	Super::GetNodeContextMenuActions(Menu, Context);

	if (Context->bIsDebugging)
	{
		return;
	}

	const FText ShowExecPinsTitle = LOCTEXT("ShowExecPins", "Show Exec Pins");
	const FText HideExecPinsTitle = LOCTEXT("HideExecPins", "Hide Exec Pins");
	const FText ShowExecPinsTooltip =
		LOCTEXT("ShowExecPinsTooltip", "Adds execution pins and evaluates the node when execution reaches it.");
	const FText HideExecPinsTooltip =
		LOCTEXT("HideExecPinsTooltip", "Removes execution pins and evaluates the node whenever its result is needed.");

	FToolMenuSection& Section = Menu->AddSection(
		TEXT("FlecsEntityCheck"),
		LOCTEXT("FlecsEntityCheckHeader", "Flecs Entity"));
	Section.AddMenuEntry(
		TEXT("ToggleExecPins"),
		IsNodePure() ? ShowExecPinsTitle : HideExecPinsTitle,
		IsNodePure() ? ShowExecPinsTooltip : HideExecPinsTooltip,
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateUObject(
				const_cast<UK2Node_FlecsGenericEntityCheck*>(this),
				&UK2Node_FlecsGenericEntityCheck::ToggleExecPins),
			FCanExecuteAction::CreateUObject(
				const_cast<UK2Node_FlecsGenericEntityCheck*>(this),
				&UK2Node_FlecsGenericEntityCheck::CanToggleExecPins),
			FIsActionChecked()));
}

void UK2Node_FlecsGenericEntityCheck::ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins)
{
	Super::ReallocatePinsDuringReconstruction(OldPins);
	ReconnectPureExecPins(OldPins);
}

UEdGraphPin* UK2Node_FlecsGenericEntityCheck::GetEntityPin() const
{
	return FindPin(TEXT("Entity"), EGPD_Input);
}

UEdGraphPin* UK2Node_FlecsGenericEntityCheck::GetResultPin() const
{
	return FindPin(UEdGraphSchema_K2::PN_ReturnValue, EGPD_Output);
}

void UK2Node_FlecsGenericEntityCheck::ConfigureIntermediateFunctionPurity(
	UK2Node_CallFunction& Function) const
{
	UK2Node_FlecsCallFunction* FlecsFunction = Cast<UK2Node_FlecsCallFunction>(&Function);
	solid_check(FlecsFunction);
	FlecsFunction->SetForceImpure(!IsNodePure());
}

void UK2Node_FlecsGenericEntityCheck::MoveExecLinksToIntermediate(
	FKismetCompilerContext& CompilerContext,
	UK2Node_CallFunction& Function) const
{
	if (IsNodePure())
	{
		return;
	}

	CompilerContext.MovePinLinksToIntermediate(*GetExecPin(), *Function.GetExecPin());
	CompilerContext.MovePinLinksToIntermediate(*GetThenPin(), *Function.GetThenPin());
}

void UK2Node_FlecsGenericEntityCheck::ToggleExecPins()
{
	const FScopedTransaction Transaction(
		IsNodePure()
			? LOCTEXT("ShowExecPinsTransaction", "Show Flecs Entity Node Exec Pins")
			: LOCTEXT("HideExecPinsTransaction", "Hide Flecs Entity Node Exec Pins"));
	Modify();

	switch (PurityOverride)
	{
	case ENodePurityOverride::Unset:
	case ENodePurityOverride::Pure:
		PurityOverride = ENodePurityOverride::Impure;
		break;

	case ENodePurityOverride::Impure:
		PurityOverride = ENodePurityOverride::Pure;
		break;

	default:
		solid_check(false);
	}

	ReconstructNode();
}

bool UK2Node_FlecsGenericEntityCheck::CanToggleExecPins() const
{
	if (!IsNodePure())
	{
		return true;
	}

	const UEdGraphSchema_K2* K2Schema = Cast<UEdGraphSchema_K2>(GetSchema());
	return K2Schema && K2Schema->DoesGraphSupportImpureFunctions(GetGraph());
}

bool UK2Node_FlecsGenericEntityCheck::ReconnectPureExecPins(TArray<UEdGraphPin*>& OldPins)
{
	if (!IsNodePure())
	{
		return false;
	}

	UEdGraphPin* OldExecPin = nullptr;
	UEdGraphPin* OldThenPin = nullptr;
	for (UEdGraphPin* OldPin : OldPins)
	{
		if (OldPin->PinName == UEdGraphSchema_K2::PN_Execute)
		{
			OldExecPin = OldPin;
		}
		else if (OldPin->PinName == UEdGraphSchema_K2::PN_Then)
		{
			OldThenPin = OldPin;
		}
	}

	if (!OldExecPin || !OldThenPin)
	{
		return false;
	}

	OldExecPin->SetSavePinIfOrphaned(false);
	OldThenPin->SetSavePinIfOrphaned(false);

	if (OldThenPin->LinkedTo.IsEmpty())
	{
		return false;
	}

	UEdGraphPin* ThenLinkedPin = OldThenPin->LinkedTo[0];
	while (!OldExecPin->LinkedTo.IsEmpty())
	{
		UEdGraphPin* ExecLinkedPin = OldExecPin->LinkedTo[0];
		ExecLinkedPin->BreakLinkTo(OldExecPin);
		ExecLinkedPin->MakeLinkTo(ThenLinkedPin);
	}

	return true;
}

#undef LOCTEXT_NAMESPACE
