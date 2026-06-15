// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Nodes/K2Node_FlecsEntityAddPairOperation.h"

#include "EdGraphSchema_K2.h"
#include "K2Node_CallFunction.h"
#include "KismetCompiler.h"

#include "Libraries/FlecsEntityHandleFunctionLibrary.h"

#include "Nodes/FlecsGenericNodeTypes.h"
#include "Nodes/PinLogic/FlecsGenericInputPins.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_FlecsEntityAddPairOperation)

namespace UE::Flecs::AddPairNode
{
	struct FResolverSpec
	{
		FName FunctionName;
		FName ValueParameterName;
	}; // struct FResolverSpec

	TOptional<FResolverSpec> GetResolverSpec(const EFlecsBlueprintGenericPinTypes Type)
	{
		switch (Type)
		{
		case EFlecsBlueprintGenericPinTypes::Id:
			return FResolverSpec{
				GET_FUNCTION_NAME_CHECKED(UFlecsEntityHandleFunctionLibrary, EntityView_ResolveId),
				TEXT("Id")
			};
		case EFlecsBlueprintGenericPinTypes::ScriptStruct:
			return FResolverSpec{
				GET_FUNCTION_NAME_CHECKED(UFlecsEntityHandleFunctionLibrary, EntityView_ResolveScriptStructId),
				TEXT("ScriptStruct")
			};
		case EFlecsBlueprintGenericPinTypes::Enum:
			return FResolverSpec{
				GET_FUNCTION_NAME_CHECKED(UFlecsEntityHandleFunctionLibrary, EntityView_ResolveEnumId),
				TEXT("Enum")
			};
		case EFlecsBlueprintGenericPinTypes::EnumConstant:
			return FResolverSpec{
				GET_FUNCTION_NAME_CHECKED(UFlecsEntityHandleFunctionLibrary, EntityView_ResolveEnumConstantId),
				TEXT("EnumConstant")
			};
		case EFlecsBlueprintGenericPinTypes::GameplayTag:
			return FResolverSpec{
				GET_FUNCTION_NAME_CHECKED(UFlecsEntityHandleFunctionLibrary, EntityView_ResolveGameplayTagId),
				TEXT("GameplayTag")
			};
		default:
			return {};
		}
	}

	static UK2Node_CallFunction* SpawnResolver(
		FKismetCompilerContext& CompilerContext,
		UK2Node* SourceNode,
		UEdGraph* SourceGraph,
		const FResolverSpec& ResolverSpec)
	{
		UK2Node_CallFunction* ResolveFunction =
			CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(SourceNode, SourceGraph);
		ResolveFunction->FunctionReference.SetExternalMember(
			ResolverSpec.FunctionName,
			UFlecsEntityHandleFunctionLibrary::StaticClass());
		ResolveFunction->AllocateDefaultPins();
		return ResolveFunction;
	}
	
} // namespace UE::Flecs::AddPairNode

void UK2Node_FlecsEntityAddPairOperation::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();
	
	GetRelationPins().AllocatePins(this);
	GetTargetPins().AllocatePins(this, EFlecsBlueprintGenericPinTypes::Id);
}

void UK2Node_FlecsEntityAddPairOperation::ExpandNode(
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

	const TOptional<UE::Flecs::AddPairNode::FResolverSpec> RelationResolverSpec =
		UE::Flecs::AddPairNode::GetResolverSpec(RelationType.GetValue());
	const TOptional<UE::Flecs::AddPairNode::FResolverSpec> TargetResolverSpec =
		UE::Flecs::AddPairNode::GetResolverSpec(TargetType.GetValue());

	if (!RelationResolverSpec.IsSet() || !TargetResolverSpec.IsSet())
	{
		Message_Error(TEXT("Pair elements must resolve to individual Flecs ids and cannot be composite pairs."));
		BreakAllNodeLinks();
		
		return;
	}

	const TSolidNotNull<UK2Node_CallFunction*> RelationResolveFunction =
		UE::Flecs::AddPairNode::SpawnResolver(
			CompilerContext,
			this,
			SourceGraph,
			RelationResolverSpec.GetValue());
	const TSolidNotNull<UK2Node_CallFunction*> TargetResolveFunction =
		UE::Flecs::AddPairNode::SpawnResolver(
			CompilerContext,
			this,
			SourceGraph,
			TargetResolverSpec.GetValue());

	const TSolidNotNull<UK2Node_CallFunction*> AddPairFunction =
		CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	AddPairFunction->FunctionReference.SetExternalMember(
		GET_FUNCTION_NAME_CHECKED(UFlecsEntityHandleFunctionLibrary, EntityHandle_AddPairIds),
		UFlecsEntityHandleFunctionLibrary::StaticClass());
	AddPairFunction->AllocateDefaultPins();

	CompilerContext.MovePinLinksToIntermediate(*GetExecPin(), *AddPairFunction->GetExecPin());
	CompilerContext.MovePinLinksToIntermediate(*GetThenPin(), *AddPairFunction->GetThenPin());
	CompilerContext.CopyPinLinksToIntermediate(
		*GetEntityPin(),
		*RelationResolveFunction->FindPinChecked(TEXT("Entity")));
	CompilerContext.CopyPinLinksToIntermediate(
		*GetEntityPin(),
		*TargetResolveFunction->FindPinChecked(TEXT("Entity")));
	CompilerContext.MovePinLinksToIntermediate(
		*GetEntityPin(),
		*AddPairFunction->FindPinChecked(TEXT("Entity")));
	CompilerContext.MovePinLinksToIntermediate(
		*RelationValuePin,
		*RelationResolveFunction->FindPinChecked(RelationResolverSpec.GetValue().ValueParameterName));
	CompilerContext.MovePinLinksToIntermediate(
		*TargetValuePin,
		*TargetResolveFunction->FindPinChecked(TargetResolverSpec.GetValue().ValueParameterName));

	const TSolidNotNull<const UEdGraphSchema_K2*> GraphSchema = GetDefault<UEdGraphSchema_K2>();
	const bool bConnectedRelation = GraphSchema->TryCreateConnection(
		RelationResolveFunction->FindPinChecked(UEdGraphSchema_K2::PN_ReturnValue),
		AddPairFunction->FindPinChecked(TEXT("RelationId")));
	const bool bConnectedTarget = GraphSchema->TryCreateConnection(
		TargetResolveFunction->FindPinChecked(UEdGraphSchema_K2::PN_ReturnValue),
		AddPairFunction->FindPinChecked(TEXT("TargetId")));

	if (!bConnectedRelation || !bConnectedTarget)
	{
		Message_Error(TEXT("Failed to connect the resolved Flecs pair ids to the Add Pair operation."));
	}

	BreakAllNodeLinks();
}

FText UK2Node_FlecsEntityAddPairOperation::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Add Pair to Flecs Entity"));
}

void UK2Node_FlecsEntityAddPairOperation::PinDefaultValueChanged(UEdGraphPin* Pin)
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

void UK2Node_FlecsEntityAddPairOperation::PostReconstructNode()
{
	Super::PostReconstructNode();
	
	GetRelationPins().RefreshPins(this);
	GetTargetPins().RefreshPins(this);
}

const FFlecsGenericInputPins& UK2Node_FlecsEntityAddPairOperation::GetRelationPins()
{
	static const FFlecsGenericInputPins RelationPins(TEXT("Relation"));
	return RelationPins;
}

const FFlecsGenericInputPins& UK2Node_FlecsEntityAddPairOperation::GetTargetPins()
{
	static const FFlecsGenericInputPins TargetPins(TEXT("Target"));
	return TargetPins;
}
