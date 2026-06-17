// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Nodes/K2Node_FlecsEntityGetOperations.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(K2Node_FlecsEntityGetOperations)

FText UK2Node_FlecsEntityGetOperation::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Get Component from Flecs Entity"));
}

FText UK2Node_FlecsEntityGetRefOperation::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Get Component Ref from Flecs Entity"));
}

FText UK2Node_FlecsEntityGetPairOperation::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Get Pair from Flecs Entity"));
}

FText UK2Node_FlecsEntityGetPairRefOperation::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Get Pair Ref from Flecs Entity"));
}

FText UK2Node_FlecsEntityGetUntypedComponentRefOperation::GetNodeTitle(
	ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Get Flecs Untyped Component Ref from Flecs Entity"));
}

FText UK2Node_FlecsEntityGetPairUntypedComponentRefOperation::GetNodeTitle(
	ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Get Flecs Untyped Pair Ref from Flecs Entity"));
}
