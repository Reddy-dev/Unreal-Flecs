// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "K2Node_CallFunction.h"

#include "K2Node_FlecsGenericBase.h"

#include "K2Node_FlecsGenericEntityCheck.generated.h"

class FKismetCompilerContext;
UCLASS(Abstract)
class UNREALFLECSUNCOOKED_API UK2Node_FlecsGenericEntityCheck : public UK2Node_FlecsGenericBase
{
	GENERATED_BODY()

public:
	// UK2Node interface
	virtual void AllocateDefaultPins() override;
	virtual void GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const override;
	
	virtual bool IsNodePure() const override
	{
		return PurityOverride != ENodePurityOverride::Impure;
	}
	
	virtual void ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins) override;
	// End of UK2Node interface

	NO_DISCARD UEdGraphPin* GetEntityPin() const;
	NO_DISCARD UEdGraphPin* GetResultPin() const;

protected:
	void ConfigureIntermediateFunctionPurity(UK2Node_CallFunction& Function) const;
	void MoveExecLinksToIntermediate(
		FKismetCompilerContext& CompilerContext,
		UK2Node_CallFunction& Function) const;

private:
	void ToggleExecPins();
	bool CanToggleExecPins() const;
	bool ReconnectPureExecPins(TArray<UEdGraphPin*>& OldPins);

	UPROPERTY()
	ENodePurityOverride PurityOverride = ENodePurityOverride::Unset;

}; // class UK2Node_FlecsGenericEntityCheck
