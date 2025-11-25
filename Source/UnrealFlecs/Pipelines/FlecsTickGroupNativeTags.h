// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

struct UNREALFLECS_API FFlecsTickGroupNativeTags : public FGameplayTagNativeAdder
{
public:
	FGameplayTag FlecsTickGroupRoot;
	FGameplayTag FlecsTickGroup_PrePhysics;
	FGameplayTag FlecsTickGroup_UnpausedPrePhysics;
	FGameplayTag FlecsTickGroup_DuringPhysics;
	FGameplayTag FlecsTickGroup_PostPhysics;
	FGameplayTag FlecsTickGroup_UnpausedPostPhysics;
	FGameplayTag FlecsTickGroup_PostUpdate;
	FGameplayTag FlecsTickGroupLast;
	
	virtual void AddTags() override;

	static FFlecsTickGroupNativeTags& Get();
	
}; // struct FFlecsTickGroupNativeTags
