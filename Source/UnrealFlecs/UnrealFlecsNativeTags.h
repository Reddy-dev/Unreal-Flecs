// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "GameplayTagContainer.h"

struct UNREALFLECS_API FFlecsNativeTags : public FGameplayTagNativeAdder
{
public:
	FGameplayTag FlecsRootTag;
	
	virtual void AddTags() override;

	static FFlecsNativeTags& Get();
	
}; // struct FFlecsNativeTags
