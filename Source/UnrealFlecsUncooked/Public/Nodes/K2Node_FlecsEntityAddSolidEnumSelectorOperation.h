// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Nodes/Base/K2Node_FlecsGenericEntityOperation.h"

#include "K2Node_FlecsEntityAddSolidEnumSelectorOperation.generated.h"

UCLASS()
class UNREALFLECSUNCOOKED_API UK2Node_FlecsEntityAddSolidEnumSelectorOperation : public UK2Node_FlecsGenericEntityOperation
{
	GENERATED_BODY()

public:
	virtual void AllocateDefaultPins() override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

private:
	NO_DISCARD UEdGraphPin* GetEnumSelectorPin() const;

}; // class UK2Node_FlecsEntityAddSolidEnumSelectorOperation
