// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Types/SolidNotNull.h"

#include "Nodes/FlecsGenericNodeTypes.h"

class UEdGraphPin;
class UK2Node;

struct UNREALFLECSUNCOOKED_API FFlecsGenericInputPins
{
	FORCEINLINE explicit FFlecsGenericInputPins(
		const FName InPrefix = NAME_None,
		const FName InSelectorPinName = NAME_None)
		: Prefix(InPrefix),
		  SelectorPinName(InSelectorPinName)
	{
	}

	void AllocatePins(TSolidNotNull<UK2Node*> Node,
		EFlecsBlueprintGenericPinTypes DefaultType = EFlecsBlueprintGenericPinTypes::ScriptStruct) const;

	void RefreshPins(TSolidNotNull<UK2Node*> Node) const;
	void SetFriendlyNames(
		TSolidNotNull<UK2Node*> Node,
		const FText& SelectorName,
		const FText& ValueName) const;
	void SetPinsHidden(TSolidNotNull<UK2Node*> Node, bool bHidden) const;

	NO_DISCARD bool IsSelectorPin(TSolidNotNull<const UK2Node*> Node, TSolidNotNull<const UEdGraphPin*> Pin) const;
	NO_DISCARD bool IsManagedPin(TSolidNotNull<const UK2Node*> Node, TSolidNotNull<const UEdGraphPin*> Pin) const;
	NO_DISCARD TOptional<EFlecsBlueprintGenericPinTypes> GetSelectedType(TSolidNotNull<const UK2Node*> Node) const;

	NO_DISCARD UEdGraphPin* GetSelectorPin(TSolidNotNull<const UK2Node*> Node) const;
	NO_DISCARD UEdGraphPin* GetValuePin(TSolidNotNull<const UK2Node*> Node, EFlecsBlueprintGenericPinTypes Type) const;
	NO_DISCARD UEdGraphPin* GetActiveValuePin(TSolidNotNull<const UK2Node*> Node) const;

	NO_DISCARD FName GetSelectorPinName() const;
	NO_DISCARD FName GetValuePinName(EFlecsBlueprintGenericPinTypes Type) const;

private:
	NO_DISCARD FName MakePinName(const TCHAR* Suffix) const;

	FName Prefix;
	FName SelectorPinName;

}; // struct FFlecsGenericInputPins
