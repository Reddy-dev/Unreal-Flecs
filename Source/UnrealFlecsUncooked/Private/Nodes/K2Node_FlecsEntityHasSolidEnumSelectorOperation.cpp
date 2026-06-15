// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Nodes/K2Node_FlecsEntityHasSolidEnumSelectorOperation.h"

#include "EdGraphSchema_K2.h"
#include "KismetCompiler.h"

#include "Libraries/FlecsEntityHandleFunctionLibrary.h"
#include "Types/SolidEnumSelector.h"
#include "Types/SolidNotNull.h"

#include "Nodes/K2Node_FlecsCallFunction.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_FlecsEntityHasSolidEnumSelectorOperation)

void UK2Node_FlecsEntityHasSolidEnumSelectorOperation::AllocateDefaultPins()
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

void UK2Node_FlecsEntityHasSolidEnumSelectorOperation::ExpandNode(
	FKismetCompilerContext& CompilerContext,
	UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	const TSolidNotNull<UK2Node_FlecsCallFunction*> HasFunction =
		CompilerContext.SpawnIntermediateNode<UK2Node_FlecsCallFunction>(this, SourceGraph);
	HasFunction->FunctionReference.SetExternalMember(
		GET_FUNCTION_NAME_CHECKED(
			UFlecsEntityHandleFunctionLibrary,
			EntityView_HasSolidEnumSelector),
		UFlecsEntityHandleFunctionLibrary::StaticClass());
	ConfigureIntermediateFunctionPurity(*HasFunction);
	HasFunction->AllocateDefaultPins();

	MoveExecLinksToIntermediate(CompilerContext, *HasFunction);
	CompilerContext.MovePinLinksToIntermediate(
		*GetEntityPin(),
		*HasFunction->FindPinChecked(TEXT("Entity")));
	CompilerContext.MovePinLinksToIntermediate(
		*GetEnumSelectorPin(),
		*HasFunction->FindPinChecked(TEXT("EnumSelector")));
	CompilerContext.MovePinLinksToIntermediate(
		*GetResultPin(),
		*HasFunction->FindPinChecked(UEdGraphSchema_K2::PN_ReturnValue));

	BreakAllNodeLinks();
}

FText UK2Node_FlecsEntityHasSolidEnumSelectorOperation::GetNodeTitle(
	ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Has Solid Enum Selector on Flecs Entity"));
}

UEdGraphPin* UK2Node_FlecsEntityHasSolidEnumSelectorOperation::GetEnumSelectorPin() const
{
	return FindPin(TEXT("EnumSelector"), EGPD_Input);
}
