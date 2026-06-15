// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "K2Node.h"

#include "SolidMacros/Macros.h"

#include "K2Node_FlecsGenericBase.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class UNREALFLECSUNCOOKED_API UK2Node_FlecsGenericBase : public UK2Node
{
	GENERATED_BODY()

public:
	
// UK2Node interface
	virtual bool NodeCausesStructuralBlueprintChange() const override { return true; }
	virtual bool ShouldShowNodeProperties() const override { return false; }
	virtual FText GetMenuCategory() const override;
	virtual bool IsNodePure() const override { return true; }
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
// End of UK2Node interface
	
}; // class UK2Node_FlecsGenericBase
