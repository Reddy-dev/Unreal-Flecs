// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "FlecsTickGroupNativeTags.h"

#include "GameplayTagsManager.h"

void FFlecsTickGroupNativeTags::AddTags()
{
	UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();

	FlecsTickGroupRoot = TagsManager.AddNativeGameplayTag(
		FName("Flecs.TickGroup"), TEXT("Root tag for all Flecs Tick Groups"));

	FlecsTickGroup_PrePhysics = TagsManager.AddNativeGameplayTag(
		FName("Flecs.TickGroup.Unreal.PrePhysics"), TEXT("Flecs Tick Group for Pre-Physics Ticking"));

	FlecsTickGroup_UnpausedPrePhysics = TagsManager.AddNativeGameplayTag(
		FName("Flecs.TickGroup.Unreal.PrePhysics.Unpaused"), TEXT("Flecs Tick Group for Unpaused Pre-Physics Ticking"));

	FlecsTickGroup_DuringPhysics = TagsManager.AddNativeGameplayTag(
		FName("Flecs.TickGroup.Unreal.DuringPhysics"), TEXT("Flecs Tick Group for During-Physics Ticking"));

	FlecsTickGroup_PostPhysics = TagsManager.AddNativeGameplayTag(
		FName("Flecs.TickGroup.Unreal.PostPhysics"), TEXT("Flecs Tick Group for Post-Physics Ticking"));

	FlecsTickGroup_UnpausedPostPhysics = TagsManager.AddNativeGameplayTag(
		FName("Flecs.TickGroup.Unreal.PreUpdate"), TEXT("Flecs Tick Group for Pre-Update Ticking"));

	FlecsTickGroup_PostUpdate = TagsManager.AddNativeGameplayTag(
		FName("Flecs.TickGroup.Unreal.PostUpdate"), TEXT("Flecs Tick Group for Post-Update Ticking"));
}

FFlecsTickGroupNativeTags& FFlecsTickGroupNativeTags::Get()
{
	static FFlecsTickGroupNativeTags Instance;
	return Instance;
}