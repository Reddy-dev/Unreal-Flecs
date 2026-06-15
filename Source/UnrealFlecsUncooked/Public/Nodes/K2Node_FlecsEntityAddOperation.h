 // Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SolidMacros/Macros.h"

#include "Nodes/Base/K2Node_FlecsGenericEntityOperation.h"

#include "K2Node_FlecsEntityAddOperation.generated.h"

struct FFlecsGenericInputPins;

 /**
  *
  */
 UCLASS()
class UNREALFLECSUNCOOKED_API UK2Node_FlecsEntityAddOperation : public UK2Node_FlecsGenericEntityOperation
{
	GENERATED_BODY()

public:
	
// UK2Node interface
	virtual void AllocateDefaultPins() override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual void ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins) override;
	virtual void PinDefaultValueChanged(UEdGraphPin* Pin) override;
	virtual void PostReconstructNode() override;
// End of UK2Node interface
	
private:
	static NO_DISCARD const FFlecsGenericInputPins& GetComponentPins();
	
}; // class UK2Node_FlecsEntityAddOperation
