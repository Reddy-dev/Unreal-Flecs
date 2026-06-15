// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Nodes/Base/K2Node_FlecsGenericEntityOperation.h"

#include "K2Node_FlecsEntitySetOperation.generated.h"

struct FFlecsGenericInputPins;

UCLASS()
class UNREALFLECSUNCOOKED_API UK2Node_FlecsEntitySetOperation : public UK2Node_FlecsGenericEntityOperation
{
	GENERATED_BODY()

public:
	// UK2Node interface
	virtual void AllocateDefaultPins() override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual bool IsConnectionDisallowed(const UEdGraphPin* MyPin, const UEdGraphPin* OtherPin, FString& OutReason) const override;
	virtual void PinConnectionListChanged(UEdGraphPin* Pin) override;
	virtual void PinDefaultValueChanged(UEdGraphPin* Pin) override;
	virtual void PinTypeChanged(UEdGraphPin* Pin) override;
	virtual void PostReconstructNode() override;
	// End of UK2Node interface

private:
	static NO_DISCARD const FFlecsGenericInputPins& GetComponentPins();
	static NO_DISCARD const FFlecsGenericInputPins& GetTargetPins();

	NO_DISCARD UEdGraphPin* GetValuePin() const;
	NO_DISCARD UEdGraphPin* GetSetPairPin() const;

	NO_DISCARD bool IsPairEnabled() const;

	void RefreshPins();
	void SynchronizeValuePinType();

}; // class UK2Node_FlecsEntitySetOperation
