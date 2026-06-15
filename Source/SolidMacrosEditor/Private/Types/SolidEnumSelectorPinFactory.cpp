// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Types/SolidEnumSelectorPinFactory.h"

#include "EdGraph/EdGraphPin.h"
#include "EdGraphSchema_K2.h"

#include "Types/SGraphPinSolidEnumSelector.h"
#include "Types/SolidEnumSelector.h"

TSharedPtr<SGraphPin> FSolidEnumSelectorPinFactory::CreatePin(UEdGraphPin* InPin) const
{
	if (InPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Struct &&
		InPin->PinType.PinSubCategoryObject == FSolidEnumSelector::StaticStruct())
	{
		return SNew(SGraphPinSolidEnumSelector, InPin);
	}

	return nullptr;
}
