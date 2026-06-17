// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Nodes/Base/K2Node_FlecsEntityHasMultipleOperation.h"

#include "K2Node_FlecsEntityHasAnyOperation.generated.h"

UCLASS()
class UNREALFLECSUNCOOKED_API UK2Node_FlecsEntityHasAnyOperation : public UK2Node_FlecsEntityHasMultipleOperation
{
	GENERATED_BODY()

public:
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

protected:
	virtual FName GetHasFunctionName() const override;

}; // class UK2Node_FlecsEntityHasAnyOperation
