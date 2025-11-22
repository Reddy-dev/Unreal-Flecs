// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "UnrealFlecsNativeTags.h"

#include "GameplayTagsManager.h"

void FFlecsNativeTags::AddTags()
{
	UGameplayTagsManager& TagManager = UGameplayTagsManager::Get();

	FlecsRootTag = TagManager.AddNativeGameplayTag(
		FName(TEXT("Flecs")),
		TEXT("Root tag for all Flecs related tags."));

	
}

FFlecsNativeTags& FFlecsNativeTags::Get()
{
	static FFlecsNativeTags Instance;
	return Instance;
}
