// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Nodes/Base/K2Node_FlecsGenericEntityOperation.h"

#include "K2Node_FlecsEntityRemoveOperation.generated.h"

struct FFlecsGenericInputPins;

UCLASS()
class UNREALFLECSUNCOOKED_API UK2Node_FlecsEntityRemoveOperation : public UK2Node_FlecsGenericEntityOperation
{
	GENERATED_BODY()

public:
	virtual void AllocateDefaultPins() override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual void PinDefaultValueChanged(UEdGraphPin* Pin) override;
	virtual void PostReconstructNode() override;

private:
	static NO_DISCARD const FFlecsGenericInputPins& GetComponentPins();

}; // class UK2Node_FlecsEntityRemoveOperation
