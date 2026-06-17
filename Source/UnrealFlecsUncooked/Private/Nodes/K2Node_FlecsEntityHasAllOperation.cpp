// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Nodes/K2Node_FlecsEntityHasAllOperation.h"

#include "Libraries/FlecsEntityHandleFunctionLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_FlecsEntityHasAllOperation)

FText UK2Node_FlecsEntityHasAllOperation::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Has All on Flecs Entity"));
}

FName UK2Node_FlecsEntityHasAllOperation::GetHasFunctionName() const
{
	return GET_FUNCTION_NAME_CHECKED(UFlecsEntityHandleFunctionLibrary, EntityView_HasAllIds);
}
