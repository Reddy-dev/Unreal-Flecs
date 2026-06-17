// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "K2Node_CallFunction.h"

#include "K2Node_FlecsCallFunction.generated.h"

UCLASS()
class UK2Node_FlecsCallFunction : public UK2Node_CallFunction
{
	GENERATED_BODY()

public:
	virtual bool IsNodePure() const override
	{
		return bForceImpure ? false : Super::IsNodePure();
	}

	void SetForceImpure(const bool bInForceImpure)
	{
		bForceImpure = bInForceImpure;
	}

private:
	bool bForceImpure = false;

}; // class UK2Node_FlecsCallFunction
