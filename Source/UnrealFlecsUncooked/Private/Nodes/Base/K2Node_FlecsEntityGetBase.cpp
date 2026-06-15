// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Nodes/Base/K2Node_FlecsEntityGetBase.h"

#include "EdGraphSchema_K2.h"
#include "Engine/Blueprint.h"
#include "Framework/Commands/UIAction.h"
#include "K2Node_CallFunction.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "KismetCompiler.h"
#include "ScopedTransaction.h"
#include "ToolMenu.h"
#include "ToolMenuSection.h"

#include "Entities/FlecsComponentRef.h"
#include "Entities/FlecsEntityView.h"
#include "Libraries/FlecsEntityHandleFunctionLibrary.h"
#include "SolidMacros/Macros.h"
#include "Types/SolidNotNull.h"

#include "Nodes/FlecsGenericNodeTypes.h"
#include "Nodes/K2Node_FlecsCallFunction.h"
#include "Nodes/PinLogic/FlecsEntityViewResolver.h"
#include "Nodes/PinLogic/FlecsGenericInputPins.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_FlecsEntityGetBase)

#define LOCTEXT_NAMESPACE "K2Node_FlecsEntityGetBase"

namespace UE::Flecs::GetNode
{
	bool IsDataBearingType(const EFlecsBlueprintGenericPinTypes Type)
	{
		return Type == EFlecsBlueprintGenericPinTypes::Id ||
			Type == EFlecsBlueprintGenericPinTypes::ScriptStruct;
	}

	UK2Node_CallFunction* SpawnFunction(
		FKismetCompilerContext& CompilerContext,
		UK2Node* SourceNode,
		UEdGraph* SourceGraph,
		const FName FunctionName,
		const bool bForceImpure = false)
	{
		UK2Node_FlecsCallFunction* Function =
			CompilerContext.SpawnIntermediateNode<UK2Node_FlecsCallFunction>(
				SourceNode,
				SourceGraph);
		Function->FunctionReference.SetExternalMember(
			FunctionName,
			UFlecsEntityHandleFunctionLibrary::StaticClass());
		Function->SetForceImpure(bForceImpure);
		Function->AllocateDefaultPins();
		return Function;
	}
} // namespace UE::Flecs::GetNode

void UK2Node_FlecsEntityGetBase::AllocateDefaultPins()
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

	GetComponentPins().AllocatePins(this);
	GetComponentPins().SetFriendlyNames(
		this,
		LOCTEXT("ComponentTypePin", "Component Type"),
		LOCTEXT("ComponentPin", "Component"));

	if (IsPairNode())
	{
		GetTargetPins().AllocatePins(this, EFlecsBlueprintGenericPinTypes::Id);
		GetTargetPins().SetFriendlyNames(
			this,
			LOCTEXT("TargetTypePin", "Target Type"),
			LOCTEXT("TargetPin", "Target"));
	}

	if (IsReferenceNode())
	{
		UEdGraphPin* ReferencePin = CreatePin(
			EGPD_Output,
			UEdGraphSchema_K2::PC_Struct,
			FFlecsComponentRef::StaticStruct(),
			TEXT("Reference"));
		ReferencePin->PinFriendlyName = LOCTEXT("ReferencePin", "Reference");
	}
	else
	{
		UEdGraphPin* ValuePin = CreatePin(
			EGPD_Output,
			UEdGraphSchema_K2::PC_Wildcard,
			TEXT("Value"));
		ValuePin->PinFriendlyName = LOCTEXT("ValuePin", "Value");
	}

	RefreshPins();
}

void UK2Node_FlecsEntityGetBase::ExpandNode(
	FKismetCompilerContext& CompilerContext,
	UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	const TOptional<EFlecsBlueprintGenericPinTypes> ComponentType =
		GetComponentPins().GetSelectedType(this);
	const TOptional<EFlecsBlueprintGenericPinTypes> TargetType = IsPairNode()
		? GetTargetPins().GetSelectedType(this)
		: TOptional<EFlecsBlueprintGenericPinTypes>();
	UEdGraphPin* ComponentValuePin = GetComponentPins().GetActiveValuePin(this);
	UEdGraphPin* ValuePin = GetValuePin();
	UScriptStruct* ValueScriptStruct = ValuePin
		? Cast<UScriptStruct>(ValuePin->PinType.PinSubCategoryObject.Get())
		: nullptr;
	UScriptStruct* SelectedComponentScriptStruct =
		!IsPairNode() &&
		ComponentType.IsSet() &&
		ComponentType.GetValue() == EFlecsBlueprintGenericPinTypes::ScriptStruct
			? Cast<UScriptStruct>(ComponentValuePin
				? ComponentValuePin->DefaultObject
				: nullptr)
			: nullptr;

	if (!ComponentType.IsSet() || !ComponentValuePin)
	{
		Message_Error(TEXT("Get requires a valid component type."));
		BreakAllNodeLinks();
		return;
	}

	if (!IsReferenceNode() && !ValueScriptStruct)
	{
		Message_Error(TEXT("Get requires a concrete component struct or Value output struct type."));
		BreakAllNodeLinks();
		return;
	}

	if (!IsReferenceNode() &&
		SelectedComponentScriptStruct &&
		SelectedComponentScriptStruct != ValueScriptStruct)
	{
		Message_Error(TEXT("Get component struct must match the Value output struct type."));
		BreakAllNodeLinks();
		return;
	}

	if (!IsPairNode() &&
		!UE::Flecs::GetNode::IsDataBearingType(ComponentType.GetValue()))
	{
		Message_Error(TEXT("A non-pair Get component must be a Flecs id or script struct."));
		BreakAllNodeLinks();
		return;
	}

	if (IsPairNode() &&
		(!TargetType.IsSet() ||
			(!UE::Flecs::GetNode::IsDataBearingType(ComponentType.GetValue()) &&
				!UE::Flecs::GetNode::IsDataBearingType(TargetType.GetValue()))))
	{
		Message_Error(TEXT("Get Pair requires at least one Flecs id or script struct side."));
		BreakAllNodeLinks();
		return;
	}

	if (!IsPairNode())
	{
		const bool bScriptStruct =
			ComponentType.GetValue() == EFlecsBlueprintGenericPinTypes::ScriptStruct;
		const FName GetFunctionName = IsReferenceNode()
			? (bScriptStruct
				? GET_FUNCTION_NAME_CHECKED(
					UFlecsEntityHandleFunctionLibrary,
					EntityView_GetRefScriptStruct)
				: GET_FUNCTION_NAME_CHECKED(
					UFlecsEntityHandleFunctionLibrary,
					EntityView_GetRefId))
			: (bScriptStruct
				? GET_FUNCTION_NAME_CHECKED(
					UFlecsEntityHandleFunctionLibrary,
					EntityView_GetScriptStruct)
				: GET_FUNCTION_NAME_CHECKED(
					UFlecsEntityHandleFunctionLibrary,
					EntityView_GetId));
		const FName ComponentParameterName =
			bScriptStruct ? TEXT("ScriptStruct") : TEXT("ComponentId");

		const TSolidNotNull<UK2Node_CallFunction*> GetFunction =
			UE::Flecs::GetNode::SpawnFunction(
				CompilerContext,
				this,
				SourceGraph,
				GetFunctionName,
				!IsNodePure());

		if (!IsNodePure())
		{
			CompilerContext.MovePinLinksToIntermediate(
				*GetExecPin(),
				*GetFunction->GetExecPin());
			CompilerContext.MovePinLinksToIntermediate(
				*GetThenPin(),
				*GetFunction->GetThenPin());
		}

		CompilerContext.MovePinLinksToIntermediate(
			*GetEntityPin(),
			*GetFunction->FindPinChecked(TEXT("Entity")));

		UEdGraphPin* FunctionComponentPin =
			GetFunction->FindPinChecked(ComponentParameterName);
		if (bScriptStruct &&
			!IsReferenceNode() &&
			ComponentValuePin->LinkedTo.IsEmpty() &&
			!ComponentValuePin->DefaultObject)
		{
			FunctionComponentPin->DefaultObject = ValueScriptStruct;
		}
		else
		{
			CompilerContext.MovePinLinksToIntermediate(
				*ComponentValuePin,
				*FunctionComponentPin);
		}

		if (IsReferenceNode())
		{
			CompilerContext.MovePinLinksToIntermediate(
				*GetReferencePin(),
				*GetFunction->FindPinChecked(UEdGraphSchema_K2::PN_ReturnValue));
		}
		else
		{
			UEdGraphPin* FunctionValuePin = GetFunction->FindPinChecked(TEXT("Value"));
			FunctionValuePin->PinType = ValuePin->PinType;
			CompilerContext.MovePinLinksToIntermediate(*ValuePin, *FunctionValuePin);
		}

		BreakAllNodeLinks();
		return;
	}

	const TOptional<UE::Flecs::EntityViewResolver::FResolverSpec> ComponentResolverSpec =
		UE::Flecs::EntityViewResolver::GetResolverSpec(ComponentType.GetValue());
	if (!ComponentResolverSpec.IsSet() || ComponentResolverSpec.GetValue().bCompositePair)
	{
		Message_Error(TEXT("Get component must resolve to one Flecs id."));
		BreakAllNodeLinks();
		return;
	}

	const TSolidNotNull<UK2Node_CallFunction*> ComponentResolver =
		UE::Flecs::EntityViewResolver::SpawnResolver(
			CompilerContext,
			this,
			SourceGraph,
			ComponentResolverSpec.GetValue());
	CompilerContext.CopyPinLinksToIntermediate(
		*GetEntityPin(),
		*ComponentResolver->FindPinChecked(TEXT("Entity")));

	if (!IsReferenceNode() &&
		ComponentType.GetValue() == EFlecsBlueprintGenericPinTypes::ScriptStruct &&
		ComponentValuePin->LinkedTo.IsEmpty() &&
		!ComponentValuePin->DefaultObject)
	{
		ComponentResolver->FindPinChecked(
			ComponentResolverSpec.GetValue().ValueParameterName)->DefaultObject =
				ValueScriptStruct;
	}
	else
	{
		CompilerContext.MovePinLinksToIntermediate(
			*ComponentValuePin,
			*ComponentResolver->FindPinChecked(
				ComponentResolverSpec.GetValue().ValueParameterName));
	}

	UEdGraphPin* FinalComponentIdPin =
		ComponentResolver->FindPinChecked(UEdGraphSchema_K2::PN_ReturnValue);

	if (IsPairNode())
	{
		UEdGraphPin* TargetValuePin = GetTargetPins().GetActiveValuePin(this);
		const TOptional<UE::Flecs::EntityViewResolver::FResolverSpec> TargetResolverSpec =
			TargetType.IsSet()
				? UE::Flecs::EntityViewResolver::GetResolverSpec(TargetType.GetValue())
				: TOptional<UE::Flecs::EntityViewResolver::FResolverSpec>();
		if (!TargetValuePin || !TargetResolverSpec.IsSet() ||
			TargetResolverSpec.GetValue().bCompositePair)
		{
			Message_Error(TEXT("Get Pair target must resolve to one Flecs id."));
			BreakAllNodeLinks();
			return;
		}

		const TSolidNotNull<UK2Node_CallFunction*> TargetResolver =
			UE::Flecs::EntityViewResolver::SpawnResolver(
				CompilerContext,
				this,
				SourceGraph,
				TargetResolverSpec.GetValue());
		CompilerContext.CopyPinLinksToIntermediate(
			*GetEntityPin(),
			*TargetResolver->FindPinChecked(TEXT("Entity")));
		CompilerContext.MovePinLinksToIntermediate(
			*TargetValuePin,
			*TargetResolver->FindPinChecked(
				TargetResolverSpec.GetValue().ValueParameterName));

		const TSolidNotNull<UK2Node_CallFunction*> MakePairFunction =
			UE::Flecs::GetNode::SpawnFunction(
				CompilerContext,
				this,
				SourceGraph,
				GET_FUNCTION_NAME_CHECKED(
					UFlecsEntityHandleFunctionLibrary,
					EntityHandle_MakePairId));

		const TSolidNotNull<const UEdGraphSchema_K2*> GraphSchema =
			GetDefault<UEdGraphSchema_K2>();
		const bool bConnectedRelation = GraphSchema->TryCreateConnection(
			FinalComponentIdPin,
			MakePairFunction->FindPinChecked(TEXT("RelationId")));
		const bool bConnectedTarget = GraphSchema->TryCreateConnection(
			TargetResolver->FindPinChecked(UEdGraphSchema_K2::PN_ReturnValue),
			MakePairFunction->FindPinChecked(TEXT("TargetId")));
		if (!bConnectedRelation || !bConnectedTarget)
		{
			Message_Error(TEXT("Failed to create the Get Pair id."));
			BreakAllNodeLinks();
			return;
		}

		FinalComponentIdPin =
			MakePairFunction->FindPinChecked(UEdGraphSchema_K2::PN_ReturnValue);
	}

	const FName GetFunctionName = IsReferenceNode()
		? GET_FUNCTION_NAME_CHECKED(
			UFlecsEntityHandleFunctionLibrary,
			EntityView_GetRefId)
		: GET_FUNCTION_NAME_CHECKED(
			UFlecsEntityHandleFunctionLibrary,
			EntityView_GetId);
	const TSolidNotNull<UK2Node_CallFunction*> GetFunction =
		UE::Flecs::GetNode::SpawnFunction(
			CompilerContext,
			this,
			SourceGraph,
			GetFunctionName,
			!IsNodePure());

	if (!IsNodePure())
	{
		CompilerContext.MovePinLinksToIntermediate(
			*GetExecPin(),
			*GetFunction->GetExecPin());
		CompilerContext.MovePinLinksToIntermediate(
			*GetThenPin(),
			*GetFunction->GetThenPin());
	}

	CompilerContext.MovePinLinksToIntermediate(
		*GetEntityPin(),
		*GetFunction->FindPinChecked(TEXT("Entity")));

	const TSolidNotNull<const UEdGraphSchema_K2*> GraphSchema =
		GetDefault<UEdGraphSchema_K2>();
	if (!GraphSchema->TryCreateConnection(
		FinalComponentIdPin,
		GetFunction->FindPinChecked(TEXT("ComponentId"))))
	{
		Message_Error(TEXT("Failed to connect the resolved Flecs id to the Get operation."));
		BreakAllNodeLinks();
		return;
	}

	if (IsReferenceNode())
	{
		CompilerContext.MovePinLinksToIntermediate(
			*GetReferencePin(),
			*GetFunction->FindPinChecked(UEdGraphSchema_K2::PN_ReturnValue));
	}
	else
	{
		UEdGraphPin* FunctionValuePin = GetFunction->FindPinChecked(TEXT("Value"));
		FunctionValuePin->PinType = ValuePin->PinType;
		CompilerContext.MovePinLinksToIntermediate(*ValuePin, *FunctionValuePin);
	}

	BreakAllNodeLinks();
}

void UK2Node_FlecsEntityGetBase::GetNodeContextMenuActions(
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
		TEXT("FlecsEntityGet"),
		LOCTEXT("FlecsEntityGetHeader", "Flecs Entity"));
	Section.AddMenuEntry(
		TEXT("ToggleExecPins"),
		IsNodePure() ? ShowExecPinsTitle : HideExecPinsTitle,
		IsNodePure() ? ShowExecPinsTooltip : HideExecPinsTooltip,
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateUObject(
				const_cast<UK2Node_FlecsEntityGetBase*>(this),
				&UK2Node_FlecsEntityGetBase::ToggleExecPins),
			FCanExecuteAction::CreateUObject(
				const_cast<UK2Node_FlecsEntityGetBase*>(this),
				&UK2Node_FlecsEntityGetBase::CanToggleExecPins),
			FIsActionChecked()));
}

bool UK2Node_FlecsEntityGetBase::IsConnectionDisallowed(
	const UEdGraphPin* MyPin,
	const UEdGraphPin* OtherPin,
	FString& OutReason) const
{
	if (MyPin == GetValuePin() &&
		OtherPin->PinType.PinCategory != UEdGraphSchema_K2::PC_Struct)
	{
		OutReason = LOCTEXT("ValueMustBeStruct", "Get Value must be a struct.").ToString();
		return true;
	}

	return Super::IsConnectionDisallowed(MyPin, OtherPin, OutReason);
}

void UK2Node_FlecsEntityGetBase::PinConnectionListChanged(UEdGraphPin* Pin)
{
	Super::PinConnectionListChanged(Pin);

	if (Pin == GetValuePin())
	{
		SynchronizeValuePinType();
		RefreshPins();
	}
}

void UK2Node_FlecsEntityGetBase::PinDefaultValueChanged(UEdGraphPin* Pin)
{
	Super::PinDefaultValueChanged(Pin);

	if (!Pin)
	{
		return;
	}

	const bool bComponentSelectorChanged = GetComponentPins().IsSelectorPin(this, Pin);
	const bool bComponentScriptStructChanged =
		GetComponentPins().GetValuePin(
			this,
			EFlecsBlueprintGenericPinTypes::ScriptStruct) == Pin;
	const bool bTargetSelectorChanged =
		IsPairNode() && GetTargetPins().IsSelectorPin(this, Pin);

	if (bComponentSelectorChanged || bComponentScriptStructChanged)
	{
		SynchronizeValuePinType();
	}

	if (bComponentSelectorChanged || bComponentScriptStructChanged || bTargetSelectorChanged)
	{
		RefreshPins();

		if (UEdGraph* Graph = GetGraph())
		{
			Graph->NotifyGraphChanged();
		}
	}
}

void UK2Node_FlecsEntityGetBase::PinTypeChanged(UEdGraphPin* Pin)
{
	Super::PinTypeChanged(Pin);

	if (Pin == GetValuePin())
	{
		SynchronizeValuePinType();
		RefreshPins();
	}
}

void UK2Node_FlecsEntityGetBase::PostReconstructNode()
{
	Super::PostReconstructNode();
	SynchronizeValuePinType();
	RefreshPins();
}

void UK2Node_FlecsEntityGetBase::ReallocatePinsDuringReconstruction(
	TArray<UEdGraphPin*>& OldPins)
{
	Super::ReallocatePinsDuringReconstruction(OldPins);
	ReconnectPureExecPins(OldPins);
}

const FFlecsGenericInputPins& UK2Node_FlecsEntityGetBase::GetComponentPins()
{
	static const FFlecsGenericInputPins ComponentPins(
		TEXT("Component"),
		TEXT("ComponentType"));
	return ComponentPins;
}

const FFlecsGenericInputPins& UK2Node_FlecsEntityGetBase::GetTargetPins()
{
	static const FFlecsGenericInputPins TargetPins(TEXT("Target"));
	return TargetPins;
}

UEdGraphPin* UK2Node_FlecsEntityGetBase::GetEntityPin() const
{
	return FindPin(TEXT("Entity"), EGPD_Input);
}

UEdGraphPin* UK2Node_FlecsEntityGetBase::GetValuePin() const
{
	return FindPin(TEXT("Value"), EGPD_Output);
}

UEdGraphPin* UK2Node_FlecsEntityGetBase::GetReferencePin() const
{
	return FindPin(TEXT("Reference"), EGPD_Output);
}

void UK2Node_FlecsEntityGetBase::ToggleExecPins()
{
	const FScopedTransaction Transaction(
		IsNodePure()
			? LOCTEXT("ShowExecPinsTransaction", "Show Flecs Get Node Exec Pins")
			: LOCTEXT("HideExecPinsTransaction", "Hide Flecs Get Node Exec Pins"));
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

bool UK2Node_FlecsEntityGetBase::CanToggleExecPins() const
{
	if (!IsNodePure())
	{
		return true;
	}

	const UEdGraphSchema_K2* K2Schema = Cast<UEdGraphSchema_K2>(GetSchema());
	return K2Schema && K2Schema->DoesGraphSupportImpureFunctions(GetGraph());
}

bool UK2Node_FlecsEntityGetBase::ReconnectPureExecPins(TArray<UEdGraphPin*>& OldPins)
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

void UK2Node_FlecsEntityGetBase::RefreshPins()
{
	if (UEdGraphPin* ComponentTypePin = GetComponentPins().GetSelectorPin(this))
	{
		ComponentTypePin->PinType.PinSubCategoryObject = IsPairNode()
			? StaticEnum<EFlecsBlueprintGenericPinTypes>()
			: StaticEnum<EFlecsBlueprintSetComponentType>();

		const TOptional<EFlecsBlueprintGenericPinTypes> ComponentType =
			GetComponentPins().GetSelectedType(this);
		if (!IsPairNode() &&
			(!ComponentType.IsSet() ||
				!UE::Flecs::GetNode::IsDataBearingType(ComponentType.GetValue())))
		{
			GetDefault<UEdGraphSchema_K2>()->TrySetDefaultValue(
				*ComponentTypePin,
				TEXT("ScriptStruct"));
		}
	}

	GetComponentPins().SetPinsHidden(this, false);

	if (IsPairNode())
	{
		GetTargetPins().SetPinsHidden(this, false);
	}
}

void UK2Node_FlecsEntityGetBase::SynchronizeValuePinType()
{
	UEdGraphPin* ValuePin = GetValuePin();
	if (!ValuePin)
	{
		return;
	}

	FEdGraphPinType NewPinType;
	NewPinType.PinCategory = UEdGraphSchema_K2::PC_Wildcard;
	if (!ValuePin->LinkedTo.IsEmpty())
	{
		NewPinType = ValuePin->LinkedTo[0]->PinType;
	}
	else
	{
		const TOptional<EFlecsBlueprintGenericPinTypes> ComponentType =
			GetComponentPins().GetSelectedType(this);
		if (!IsPairNode() &&
			ComponentType.IsSet() &&
			ComponentType.GetValue() == EFlecsBlueprintGenericPinTypes::ScriptStruct)
		{
			const UEdGraphPin* ComponentScriptStructPin =
				GetComponentPins().GetValuePin(
					this,
					EFlecsBlueprintGenericPinTypes::ScriptStruct);
			if (UScriptStruct* ScriptStruct = ComponentScriptStructPin
				? Cast<UScriptStruct>(ComponentScriptStructPin->DefaultObject)
				: nullptr)
			{
				NewPinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
				NewPinType.PinSubCategoryObject = ScriptStruct;
			}
		}
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

#undef LOCTEXT_NAMESPACE
