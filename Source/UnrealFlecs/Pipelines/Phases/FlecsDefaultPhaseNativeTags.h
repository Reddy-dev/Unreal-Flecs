// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "NativeGameplayTags.h"

struct UNREALFLECS_API FFlecsDefaultPhaseNativeTags : public FGameplayTagNativeAdder
{
	FGameplayTag OnStartPhaseTag;
	FGameplayTag PreFramePhaseTag;
	FGameplayTag OnLoadPhaseTag;
	FGameplayTag PostLoadPhaseTag;
	FGameplayTag PreUpdatePhaseTag;
	FGameplayTag OnUpdatePhaseTag;
	FGameplayTag OnValidatePhaseTag;
	FGameplayTag PostUpdatePhaseTag;
	FGameplayTag PreStorePhaseTag;
	FGameplayTag OnStorePhaseTag;
	FGameplayTag PostFramePhaseTag;
	
public:
	virtual void AddTags() override
	{
		UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
		
		OnStartPhaseTag = Manager.AddNativeGameplayTag("Flecs.Phases.OnStart",
			TEXT("Tag for the Flecs OnStart Phase"));

		PreFramePhaseTag = Manager.AddNativeGameplayTag("Flecs.Phases.PreFrame",
			TEXT("Tag for the Flecs PreFrame Phase"));

		OnLoadPhaseTag = Manager.AddNativeGameplayTag("Flecs.Phases.OnLoad",
			TEXT("Tag for the Flecs OnLoad Phase"));

		PostLoadPhaseTag = Manager.AddNativeGameplayTag("Flecs.Phases.PostLoad",
			TEXT("Tag for the Flecs PostLoad Phase"));

		PreUpdatePhaseTag = Manager.AddNativeGameplayTag("Flecs.Phases.PreUpdate",
			TEXT("Tag for the Flecs PreUpdate Phase"));

		OnUpdatePhaseTag = Manager.AddNativeGameplayTag("Flecs.Phases.OnUpdate",
			TEXT("Tag for the Flecs OnUpdate Phase"));

		OnValidatePhaseTag = Manager.AddNativeGameplayTag("Flecs.Phases.OnValidate",
			TEXT("Tag for the Flecs OnValidate Phase"));

		PostUpdatePhaseTag = Manager.AddNativeGameplayTag("Flecs.Phases.PostUpdate",
			TEXT("Tag for the Flecs PostUpdate Phase"));
		
		PreStorePhaseTag = Manager.AddNativeGameplayTag("Flecs.Phases.PreStore",
			TEXT("Tag for the Flecs PreStore Phase"));

		OnStorePhaseTag = Manager.AddNativeGameplayTag("Flecs.Phases.OnStore",
			TEXT("Tag for the Flecs OnStore Phase"));

		PostFramePhaseTag = Manager.AddNativeGameplayTag("Flecs.Phases.PostFrame",
			TEXT("Tag for the Flecs PostFrame Phase"));
	}

	static const FFlecsDefaultPhaseNativeTags& Get();
	
}; // class FFlecsDefaultPhaseNativeTags

