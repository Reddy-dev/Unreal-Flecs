// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Nodes/K2Node_FlecsEntitySetSolidEnumSelectorOperation.h"

#include "EdGraphSchema_K2.h"
#include "Engine/Blueprint.h"
#include "K2Node_CallFunction.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "KismetCompiler.h"

#include "Libraries/FlecsEntityHandleFunctionLibrary.h"
#include "Types/SolidEnumSelector.h"
#include "Types/SolidNotNull.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_FlecsEntitySetSolidEnumSelectorOperation)

void UK2Node_FlecsEntitySetSolidEnumSelectorOperation::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();

	UEdGraphNode::FCreatePinParams PinParams;
	PinParams.bIsConst = true;
	UEdGraphPin* EnumSelectorPin = CreatePin(
		EGPD_Input,
		UEdGraphSchema_K2::PC_Struct,
		TBaseStructure<FSolidEnumSelector>::Get(),
		TEXT("EnumSelector"),
		PinParams);
	EnumSelectorPin->PinFriendlyName = FText::FromString(TEXT("Solid Enum Selector"));

	UEdGraphPin* ValuePin = CreatePin(
		EGPD_Input,
		UEdGraphSchema_K2::PC_Wildcard,
		TEXT("Value"));
	ValuePin->PinFriendlyName = FText::FromString(TEXT("Value"));
}

void UK2Node_FlecsEntitySetSolidEnumSelectorOperation::ExpandNode(
	FKismetCompilerContext& CompilerContext,
	UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	UEdGraphPin* ValuePin = GetValuePin();
	if (!ValuePin || ValuePin->LinkedTo.Num() != 1 ||
		ValuePin->PinType.PinCategory != UEdGraphSchema_K2::PC_Struct)
	{
		Message_Error(TEXT("Set Solid Enum Selector requires one connected struct Value input."));
		BreakAllNodeLinks();
		return;
	}

	const TSolidNotNull<UK2Node_CallFunction*> SetFunction =
		CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	SetFunction->FunctionReference.SetExternalMember(
		GET_FUNCTION_NAME_CHECKED(
			UFlecsEntityHandleFunctionLibrary,
			EntityHandle_SetSolidEnumSelector),
		UFlecsEntityHandleFunctionLibrary::StaticClass());
	SetFunction->AllocateDefaultPins();

	UEdGraphPin* SetValuePin = SetFunction->FindPinChecked(TEXT("Value"));
	SetValuePin->PinType = ValuePin->PinType;

	CompilerContext.MovePinLinksToIntermediate(*GetExecPin(), *SetFunction->GetExecPin());
	CompilerContext.MovePinLinksToIntermediate(*GetThenPin(), *SetFunction->GetThenPin());
	CompilerContext.MovePinLinksToIntermediate(
		*GetEntityPin(),
		*SetFunction->FindPinChecked(TEXT("Entity")));
	CompilerContext.MovePinLinksToIntermediate(
		*GetEnumSelectorPin(),
		*SetFunction->FindPinChecked(TEXT("EnumSelector")));
	CompilerContext.MovePinLinksToIntermediate(*ValuePin, *SetValuePin);

	BreakAllNodeLinks();
}

FText UK2Node_FlecsEntitySetSolidEnumSelectorOperation::GetNodeTitle(
	ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Set Solid Enum Selector on Flecs Entity"));
}

bool UK2Node_FlecsEntitySetSolidEnumSelectorOperation::IsConnectionDisallowed(
	const UEdGraphPin* MyPin,
	const UEdGraphPin* OtherPin,
	FString& OutReason) const
{
	if (MyPin == GetValuePin() &&
		OtherPin->PinType.PinCategory != UEdGraphSchema_K2::PC_Struct)
	{
		OutReason = TEXT("Set Value must be a struct.");
		return true;
	}

	return Super::IsConnectionDisallowed(MyPin, OtherPin, OutReason);
}

void UK2Node_FlecsEntitySetSolidEnumSelectorOperation::PinConnectionListChanged(
	UEdGraphPin* Pin)
{
	Super::PinConnectionListChanged(Pin);

	if (Pin == GetValuePin())
	{
		SynchronizeValuePinType();
	}
}

void UK2Node_FlecsEntitySetSolidEnumSelectorOperation::PinTypeChanged(UEdGraphPin* Pin)
{
	Super::PinTypeChanged(Pin);

	if (Pin == GetValuePin())
	{
		SynchronizeValuePinType();
	}
}

void UK2Node_FlecsEntitySetSolidEnumSelectorOperation::PostReconstructNode()
{
	Super::PostReconstructNode();
	SynchronizeValuePinType();
}

UEdGraphPin* UK2Node_FlecsEntitySetSolidEnumSelectorOperation::GetEnumSelectorPin() const
{
	return FindPin(TEXT("EnumSelector"), EGPD_Input);
}

UEdGraphPin* UK2Node_FlecsEntitySetSolidEnumSelectorOperation::GetValuePin() const
{
	return FindPin(TEXT("Value"), EGPD_Input);
}

void UK2Node_FlecsEntitySetSolidEnumSelectorOperation::SynchronizeValuePinType()
{
	UEdGraphPin* ValuePin = GetValuePin();
	if (!ValuePin)
	{
		return;
	}

	FEdGraphPinType NewPinType;
	NewPinType.PinCategory = UEdGraphSchema_K2::PC_Wildcard;
	if (ValuePin->LinkedTo.Num() == 1)
	{
		NewPinType = ValuePin->LinkedTo[0]->PinType;
	}

	if (ValuePin->PinType != NewPinType)
	{
		ValuePin->PinType = NewPinType;

		if (UEdGraph* Graph = GetGraph())
		{
			Graph->NotifyNodeChanged(this);
		}

		if (UBlueprint* Blueprint = GetBlueprint(); Blueprint && !Blueprint->bBeingCompiled)
		{
			FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
		}
	}
}
