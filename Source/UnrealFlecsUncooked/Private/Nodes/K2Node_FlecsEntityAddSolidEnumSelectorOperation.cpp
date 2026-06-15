// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Nodes/K2Node_FlecsEntityAddSolidEnumSelectorOperation.h"

#include "EdGraphSchema_K2.h"
#include "K2Node_CallFunction.h"
#include "KismetCompiler.h"

#include "Libraries/FlecsEntityHandleFunctionLibrary.h"
#include "Types/SolidEnumSelector.h"
#include "Types/SolidNotNull.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_FlecsEntityAddSolidEnumSelectorOperation)

void UK2Node_FlecsEntityAddSolidEnumSelectorOperation::AllocateDefaultPins()
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
}

void UK2Node_FlecsEntityAddSolidEnumSelectorOperation::ExpandNode(
	FKismetCompilerContext& CompilerContext,
	UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	const TSolidNotNull<UK2Node_CallFunction*> AddFunction =
		CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	AddFunction->FunctionReference.SetExternalMember(
		GET_FUNCTION_NAME_CHECKED(
			UFlecsEntityHandleFunctionLibrary,
			EntityHandle_AddSolidEnumSelector),
		UFlecsEntityHandleFunctionLibrary::StaticClass());
	AddFunction->AllocateDefaultPins();

	CompilerContext.MovePinLinksToIntermediate(*GetExecPin(), *AddFunction->GetExecPin());
	CompilerContext.MovePinLinksToIntermediate(*GetThenPin(), *AddFunction->GetThenPin());
	CompilerContext.MovePinLinksToIntermediate(
		*GetEntityPin(),
		*AddFunction->FindPinChecked(TEXT("Entity")));
	CompilerContext.MovePinLinksToIntermediate(
		*GetEnumSelectorPin(),
		*AddFunction->FindPinChecked(TEXT("EnumSelector")));

	BreakAllNodeLinks();
}

FText UK2Node_FlecsEntityAddSolidEnumSelectorOperation::GetNodeTitle(
	ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Add Solid Enum Selector to Flecs Entity"));
}

UEdGraphPin* UK2Node_FlecsEntityAddSolidEnumSelectorOperation::GetEnumSelectorPin() const
{
	return FindPin(TEXT("EnumSelector"), EGPD_Input);
}
