// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "K2Node_AddPinInterface.h"

#include "Nodes/Base/K2Node_FlecsGenericEntityCheck.h"

#include "K2Node_FlecsEntityHasMultipleOperation.generated.h"

struct FFlecsGenericInputPins;

UCLASS(Abstract)
class UNREALFLECSUNCOOKED_API UK2Node_FlecsEntityHasMultipleOperation
	: public UK2Node_FlecsGenericEntityCheck
	, public IK2Node_AddPinInterface
{
	GENERATED_BODY()

public:
	// UK2Node interface
	virtual void AllocateDefaultPins() override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	virtual void GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const override;
	virtual void PinDefaultValueChanged(UEdGraphPin* Pin) override;
	virtual void PostReconstructNode() override;
	// End of UK2Node interface

	// IK2Node_AddPinInterface interface
	virtual void AddInputPin() override;
	virtual bool CanAddPin() const override;
	virtual void RemoveInputPin(UEdGraphPin* Pin) override;
	virtual bool CanRemovePin(const UEdGraphPin* Pin) const override;
	// End of IK2Node_AddPinInterface interface

protected:
	virtual FName GetHasFunctionName() const PURE_VIRTUAL(
		UK2Node_FlecsEntityHasMultipleOperation::GetHasFunctionName,
		return NAME_None;);

private:
	static constexpr int32 MinimumInputCount = 2;

	static FName GetInputPrefix(int32 InputIndex);
	static FFlecsGenericInputPins GetInputPins(int32 InputIndex);

	void AllocateInputPins(int32 InputIndex);
	NO_DISCARD int32 FindInputIndex(const UEdGraphPin* Pin) const;

	UPROPERTY()
	int32 NumInputs = MinimumInputCount;

}; // class UK2Node_FlecsEntityHasMultipleOperation
