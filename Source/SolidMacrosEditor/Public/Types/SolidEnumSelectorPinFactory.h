// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "EdGraphUtilities.h"

class FSolidEnumSelectorPinFactory : public FGraphPanelPinFactory
{
public:
	virtual TSharedPtr<SGraphPin> CreatePin(UEdGraphPin* InPin) const override;

}; // class FSolidEnumSelectorPinFactory
