// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Nodes/K2Node_FlecsEntityRemoveSolidEnumSelectorOperation.h"

#include "EdGraphSchema_K2.h"
#include "K2Node_CallFunction.h"
#include "KismetCompiler.h"

#include "Libraries/FlecsEntityHandleFunctionLibrary.h"
#include "Types/SolidEnumSelector.h"
#include "Types/SolidNotNull.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_FlecsEntityRemoveSolidEnumSelectorOperation)

void UK2Node_FlecsEntityRemoveSolidEnumSelectorOperation::AllocateDefaultPins()
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

void UK2Node_FlecsEntityRemoveSolidEnumSelectorOperation::ExpandNode(
	FKismetCompilerContext& CompilerContext,
	UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	const TSolidNotNull<UK2Node_CallFunction*> RemoveFunction =
		CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	RemoveFunction->FunctionReference.SetExternalMember(
		GET_FUNCTION_NAME_CHECKED(
			UFlecsEntityHandleFunctionLibrary,
			EntityHandle_RemoveSolidEnumSelector),
		UFlecsEntityHandleFunctionLibrary::StaticClass());
	RemoveFunction->AllocateDefaultPins();

	CompilerContext.MovePinLinksToIntermediate(*GetExecPin(), *RemoveFunction->GetExecPin());
	CompilerContext.MovePinLinksToIntermediate(*GetThenPin(), *RemoveFunction->GetThenPin());
	CompilerContext.MovePinLinksToIntermediate(
		*GetEntityPin(),
		*RemoveFunction->FindPinChecked(TEXT("Entity")));
	CompilerContext.MovePinLinksToIntermediate(
		*GetEnumSelectorPin(),
		*RemoveFunction->FindPinChecked(TEXT("EnumSelector")));

	BreakAllNodeLinks();
}

FText UK2Node_FlecsEntityRemoveSolidEnumSelectorOperation::GetNodeTitle(
	ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Remove Solid Enum Selector from Flecs Entity"));
}

UEdGraphPin* UK2Node_FlecsEntityRemoveSolidEnumSelectorOperation::GetEnumSelectorPin() const
{
	return FindPin(TEXT("EnumSelector"), EGPD_Input);
}
