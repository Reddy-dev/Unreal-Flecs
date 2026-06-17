// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Nodes/K2Node_FlecsEntityHasAnyOperation.h"

#include "Libraries/FlecsEntityHandleFunctionLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_FlecsEntityHasAnyOperation)

FText UK2Node_FlecsEntityHasAnyOperation::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Has Any on Flecs Entity"));
}

FName UK2Node_FlecsEntityHasAnyOperation::GetHasFunctionName() const
{
	return GET_FUNCTION_NAME_CHECKED(UFlecsEntityHandleFunctionLibrary, EntityView_HasAnyIds);
}
