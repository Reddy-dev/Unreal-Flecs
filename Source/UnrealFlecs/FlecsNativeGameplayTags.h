// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "GameplayTagContainer.h"
#include "GameplayTagsManager.h"
#include "SolidMacros/Macros.h"

struct UNREALFLECS_API FFlecsNativeGameplayTags : public FGameplayTagNativeAdder
{
	FGameplayTag FlecsRoot;
	FGameplayTag ObjectTypeRoot;

	FGameplayTag UObjectType;

	FGameplayTag FlecsModuleType;
	
	FGameplayTag AActorType;
	FGameplayTag UActorComponentType;
	FGameplayTag USceneComponentType;

	virtual void AddTags() override
	{
		UGameplayTagsManager& Manager = UGameplayTagsManager::Get();

		FlecsRoot = Manager.AddNativeGameplayTag(FName("Flecs"),
			TEXT("Root tag for all Flecs related tags"));

		ObjectTypeRoot = Manager.AddNativeGameplayTag(FName("Flecs.ObjectType"),
			TEXT("Root tag for all Flecs ObjectType tags"));

		UObjectType = Manager.AddNativeGameplayTag(FName("Flecs.ObjectType.UObject"),
			TEXT("Tag for all UObject Flecs types"));
		FlecsModuleType = Manager.AddNativeGameplayTag(FName("Flecs.ObjectType.UObject.FlecsModule"),
			TEXT("Tag for all Flecs Module types"));

		AActorType = Manager.AddNativeGameplayTag(FName("Flecs.ObjectType.UObject.AActor"),
			TEXT("Tag for all AActor Flecs types"));

		UActorComponentType = Manager.AddNativeGameplayTag(FName("Flecs.ObjectType.UObject.UActorComponent"),
			TEXT("Tag for all UActorComponent Flecs types"));

		USceneComponentType = Manager.AddNativeGameplayTag(FName("Flecs.ObjectType.UObject.UActorComponent.USceneComponent"),
			TEXT("Tag for all USceneComponent Flecs types"));
	}

	static const FFlecsNativeGameplayTags& Get()
	{
		return GFlecsNativeGameplayTags;
	}

private:
	static FFlecsNativeGameplayTags GFlecsNativeGameplayTags;
};

