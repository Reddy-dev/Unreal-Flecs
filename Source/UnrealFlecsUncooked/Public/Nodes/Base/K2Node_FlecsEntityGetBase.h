// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "K2Node_CallFunction.h"

#include "Nodes/Base/K2Node_FlecsGenericBase.h"

#include "K2Node_FlecsEntityGetBase.generated.h"

struct FFlecsGenericInputPins;

UCLASS(Abstract)
class UNREALFLECSUNCOOKED_API UK2Node_FlecsEntityGetBase : public UK2Node_FlecsGenericBase
{
	GENERATED_BODY()

public:
	virtual void AllocateDefaultPins() override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	virtual void GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const override;
	virtual bool IsConnectionDisallowed(
		const UEdGraphPin* MyPin,
		const UEdGraphPin* OtherPin,
		FString& OutReason) const override;
	virtual bool IsNodePure() const override
	{
		return PurityOverride != ENodePurityOverride::Impure;
	}
	virtual void PinConnectionListChanged(UEdGraphPin* Pin) override;
	virtual void PinDefaultValueChanged(UEdGraphPin* Pin) override;
	virtual void PinTypeChanged(UEdGraphPin* Pin) override;
	virtual void PostReconstructNode() override;
	virtual void ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins) override;

protected:
	virtual bool IsPairNode() const PURE_VIRTUAL(UK2Node_FlecsEntityGetBase::IsPairNode, return false;);
	virtual bool IsReferenceNode() const PURE_VIRTUAL(UK2Node_FlecsEntityGetBase::IsReferenceNode, return false;);
	virtual bool IsUntypedComponentRefNode() const { return false; }

private:
	static NO_DISCARD const FFlecsGenericInputPins& GetComponentPins();
	static NO_DISCARD const FFlecsGenericInputPins& GetTargetPins();

	NO_DISCARD UEdGraphPin* GetEntityPin() const;
	NO_DISCARD UEdGraphPin* GetValuePin() const;
	NO_DISCARD UEdGraphPin* GetReferencePin() const;

	void ToggleExecPins();
	bool CanToggleExecPins() const;
	bool ReconnectPureExecPins(TArray<UEdGraphPin*>& OldPins);
	void RefreshPins();
	void SynchronizeValuePinType();

	UPROPERTY()
	ENodePurityOverride PurityOverride = ENodePurityOverride::Unset;

}; // class UK2Node_FlecsEntityGetBase
