// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "K2Node_FlecsGenericBase.h"

#include "K2Node_FlecsGenericEntityOperation.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class UNREALFLECSUNCOOKED_API UK2Node_FlecsGenericEntityOperation : public UK2Node_FlecsGenericBase
{
	GENERATED_BODY()

public:
	// UK2Node interface
	virtual void AllocateDefaultPins() override;
	virtual bool IsNodePure() const override { return false; }
	// End of UK2Node interface
	
	virtual NO_DISCARD UEdGraphPin* GetEntityPin() const;
	
}; // class UK2Node_FlecsGenericEntityOperation
