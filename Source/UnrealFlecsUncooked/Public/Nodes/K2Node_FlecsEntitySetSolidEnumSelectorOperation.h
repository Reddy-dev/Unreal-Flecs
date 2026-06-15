// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Nodes/Base/K2Node_FlecsGenericEntityOperation.h"

#include "K2Node_FlecsEntitySetSolidEnumSelectorOperation.generated.h"

UCLASS()
class UNREALFLECSUNCOOKED_API UK2Node_FlecsEntitySetSolidEnumSelectorOperation
	: public UK2Node_FlecsGenericEntityOperation
{
	GENERATED_BODY()

public:
	virtual void AllocateDefaultPins() override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual bool IsConnectionDisallowed(
		const UEdGraphPin* MyPin,
		const UEdGraphPin* OtherPin,
		FString& OutReason) const override;
	virtual void PinConnectionListChanged(UEdGraphPin* Pin) override;
	virtual void PinTypeChanged(UEdGraphPin* Pin) override;
	virtual void PostReconstructNode() override;

private:
	NO_DISCARD UEdGraphPin* GetEnumSelectorPin() const;
	NO_DISCARD UEdGraphPin* GetValuePin() const;

	void SynchronizeValuePinType();

}; // class UK2Node_FlecsEntitySetSolidEnumSelectorOperation
