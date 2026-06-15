// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Nodes/K2Node_FlecsEntityHasPairOperation.h"

#include "EdGraphSchema_K2.h"
#include "K2Node_CallFunction.h"
#include "KismetCompiler.h"

#include "Libraries/FlecsEntityHandleFunctionLibrary.h"

#include "Nodes/K2Node_FlecsCallFunction.h"
#include "Nodes/PinLogic/FlecsEntityViewResolver.h"
#include "Nodes/PinLogic/FlecsGenericInputPins.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_FlecsEntityHasPairOperation)

void UK2Node_FlecsEntityHasPairOperation::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();
	GetRelationPins().AllocatePins(this);
	GetTargetPins().AllocatePins(this, EFlecsBlueprintGenericPinTypes::Id);
}

void UK2Node_FlecsEntityHasPairOperation::ExpandNode(
	FKismetCompilerContext& CompilerContext,
	UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	const TOptional<EFlecsBlueprintGenericPinTypes> RelationType = GetRelationPins().GetSelectedType(this);
	const TOptional<EFlecsBlueprintGenericPinTypes> TargetType = GetTargetPins().GetSelectedType(this);
	UEdGraphPin* RelationValuePin = GetRelationPins().GetActiveValuePin(this);
	UEdGraphPin* TargetValuePin = GetTargetPins().GetActiveValuePin(this);
	if (!RelationType.IsSet() || !TargetType.IsSet() || !RelationValuePin || !TargetValuePin)
	{
		Message_Error(TEXT("The Flecs pair input types are invalid."));
		BreakAllNodeLinks();
		return;
	}

	const TOptional<UE::Flecs::EntityViewResolver::FResolverSpec> RelationResolverSpec =
		UE::Flecs::EntityViewResolver::GetResolverSpec(RelationType.GetValue());
	const TOptional<UE::Flecs::EntityViewResolver::FResolverSpec> TargetResolverSpec =
		UE::Flecs::EntityViewResolver::GetResolverSpec(TargetType.GetValue());
	if (!RelationResolverSpec.IsSet() || !TargetResolverSpec.IsSet() ||
		RelationResolverSpec.GetValue().bCompositePair ||
		TargetResolverSpec.GetValue().bCompositePair)
	{
		Message_Error(TEXT("Pair elements must resolve to individual Flecs ids and cannot be composite pairs."));
		BreakAllNodeLinks();
		return;
	}

	const TSolidNotNull<UK2Node_CallFunction*> RelationResolveFunction =
		UE::Flecs::EntityViewResolver::SpawnResolver(
			CompilerContext,
			this,
			SourceGraph,
			RelationResolverSpec.GetValue());
	const TSolidNotNull<UK2Node_CallFunction*> TargetResolveFunction =
		UE::Flecs::EntityViewResolver::SpawnResolver(
			CompilerContext,
			this,
			SourceGraph,
			TargetResolverSpec.GetValue());

	const TSolidNotNull<UK2Node_FlecsCallFunction*> HasPairFunction =
		CompilerContext.SpawnIntermediateNode<UK2Node_FlecsCallFunction>(this, SourceGraph);
	HasPairFunction->FunctionReference.SetExternalMember(
		GET_FUNCTION_NAME_CHECKED(UFlecsEntityHandleFunctionLibrary, EntityView_HasPairIds),
		UFlecsEntityHandleFunctionLibrary::StaticClass());
	ConfigureIntermediateFunctionPurity(*HasPairFunction);
	HasPairFunction->AllocateDefaultPins();

	MoveExecLinksToIntermediate(CompilerContext, *HasPairFunction);
	CompilerContext.CopyPinLinksToIntermediate(
		*GetEntityPin(),
		*RelationResolveFunction->FindPinChecked(TEXT("Entity")));
	CompilerContext.CopyPinLinksToIntermediate(
		*GetEntityPin(),
		*TargetResolveFunction->FindPinChecked(TEXT("Entity")));
	CompilerContext.MovePinLinksToIntermediate(
		*GetEntityPin(),
		*HasPairFunction->FindPinChecked(TEXT("Entity")));
	CompilerContext.MovePinLinksToIntermediate(
		*RelationValuePin,
		*RelationResolveFunction->FindPinChecked(RelationResolverSpec.GetValue().ValueParameterName));
	CompilerContext.MovePinLinksToIntermediate(
		*TargetValuePin,
		*TargetResolveFunction->FindPinChecked(TargetResolverSpec.GetValue().ValueParameterName));
	CompilerContext.MovePinLinksToIntermediate(
		*GetResultPin(),
		*HasPairFunction->FindPinChecked(UEdGraphSchema_K2::PN_ReturnValue));

	const TSolidNotNull<const UEdGraphSchema_K2*> GraphSchema = GetDefault<UEdGraphSchema_K2>();
	const bool bConnectedRelation = GraphSchema->TryCreateConnection(
		RelationResolveFunction->FindPinChecked(UEdGraphSchema_K2::PN_ReturnValue),
		HasPairFunction->FindPinChecked(TEXT("RelationId")));
	const bool bConnectedTarget = GraphSchema->TryCreateConnection(
		TargetResolveFunction->FindPinChecked(UEdGraphSchema_K2::PN_ReturnValue),
		HasPairFunction->FindPinChecked(TEXT("TargetId")));
	if (!bConnectedRelation || !bConnectedTarget)
	{
		Message_Error(TEXT("Failed to connect the resolved Flecs pair ids to the Has Pair operation."));
	}

	BreakAllNodeLinks();
}

FText UK2Node_FlecsEntityHasPairOperation::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Has Pair on Flecs Entity"));
}

void UK2Node_FlecsEntityHasPairOperation::PinDefaultValueChanged(UEdGraphPin* Pin)
{
	Super::PinDefaultValueChanged(Pin);

	if (Pin && (GetRelationPins().IsSelectorPin(this, Pin) || GetTargetPins().IsSelectorPin(this, Pin)))
	{
		GetRelationPins().RefreshPins(this);
		GetTargetPins().RefreshPins(this);

		if (UEdGraph* Graph = GetGraph())
		{
			Graph->NotifyGraphChanged();
		}
	}
}

void UK2Node_FlecsEntityHasPairOperation::PostReconstructNode()
{
	Super::PostReconstructNode();
	
	GetRelationPins().RefreshPins(this);
	GetTargetPins().RefreshPins(this);
}

const FFlecsGenericInputPins& UK2Node_FlecsEntityHasPairOperation::GetRelationPins()
{
	static const FFlecsGenericInputPins RelationPins(TEXT("Relation"));
	return RelationPins;
}

const FFlecsGenericInputPins& UK2Node_FlecsEntityHasPairOperation::GetTargetPins()
{
	static const FFlecsGenericInputPins TargetPins(TEXT("Target"));
	return TargetPins;
}
