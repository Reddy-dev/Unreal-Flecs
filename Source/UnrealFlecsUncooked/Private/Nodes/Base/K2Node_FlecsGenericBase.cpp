// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Nodes/Base/K2Node_FlecsGenericBase.h"

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"

#include "Types/SolidNotNull.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_FlecsGenericBase)

FText UK2Node_FlecsGenericBase::GetMenuCategory() const
{
	return FText::FromString(TEXT("Flecs | Entity"));
}

void UK2Node_FlecsGenericBase::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UClass* ActionKey = GetClass();
	
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		const TSolidNotNull<UBlueprintNodeSpawner*> Spawner = UBlueprintNodeSpawner::Create(GetClass());

		ActionRegistrar.AddBlueprintAction(ActionKey, Spawner);
	}
}
