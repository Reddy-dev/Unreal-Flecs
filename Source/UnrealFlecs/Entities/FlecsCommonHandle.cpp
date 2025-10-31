﻿// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "FlecsCommonHandle.h"

#include "Engine/World.h"

#include "Worlds/FlecsWorld.h"
#include "Worlds/FlecsWorldConverter.h"
#include "Worlds/UnrealFlecsWorldTag.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsCommonHandle)

FFlecsCommonHandle::FFlecsCommonHandle(const TSolidNotNull<const UFlecsWorld*> InWorld, const FFlecsId InEntity)
{
	SetEntity(flecs::entity(InWorld->World, InEntity));
}

FFlecsCommonHandle::FFlecsCommonHandle(const flecs::world_t* InWorld, const FFlecsId InEntity)
{
	SetEntity(flecs::entity(InWorld, InEntity));
}

flecs::world FFlecsCommonHandle::GetNativeFlecsWorld() const
{
	return Entity.world();
}

UFlecsWorld* FFlecsCommonHandle::GetFlecsWorld() const
{
	if UNLIKELY_IF(!IsUnrealFlecsWorld())
	{
		return nullptr;
	}
	
	return Unreal::Flecs::ToUnrealFlecsWorld(GetEntity().world());
}

TSolidNotNull<UFlecsWorld*> FFlecsCommonHandle::GetFlecsWorldChecked() const
{
	solid_checkf(IsUnrealFlecsWorld(), TEXT("Entity is not in an Unreal Flecs World"));
	return Unreal::Flecs::ToUnrealFlecsWorld(GetEntity().world());
}

bool FFlecsCommonHandle::IsUnrealFlecsWorld() const
{
	return GetNativeFlecsWorld().has<FUnrealFlecsWorldTag>();
}

TSolidNotNull<UWorld*> FFlecsCommonHandle::GetOuterWorld() const
{
	solid_checkf(IsUnrealFlecsWorld(), TEXT("Entity is not in an Unreal Flecs World"));
	solid_checkf(::IsValid(GetFlecsWorld()), TEXT("Flecs World not found"));
	
	UWorld* World = GetFlecsWorld()->GetWorld();
	solid_checkf(::IsValid(World), TEXT("Outer UWorld not found"));
	return World;
}

FString FFlecsCommonHandle::GetWorldName() const
{
	return GetFlecsWorldChecked()->GetWorldName();
}

FFlecsId FFlecsCommonHandle::ObtainComponentTypeStruct(const TSolidNotNull<const UScriptStruct*> StructType) const
{
	return GetFlecsWorldChecked()->ObtainComponentTypeStruct(StructType);
}

FFlecsId FFlecsCommonHandle::ObtainComponentTypeEnum(const TSolidNotNull<const UEnum*> EnumType) const
{
	return GetFlecsWorldChecked()->ObtainComponentTypeEnum(EnumType);
}

FFlecsId FFlecsCommonHandle::ObtainTypeClass(const TSolidNotNull<UClass*> ClassType) const
{
	return GetFlecsWorldChecked()->ObtainTypedEntity(ClassType);
}

FFlecsId FFlecsCommonHandle::GetTagEntity(const FGameplayTag& InTag) const
{
	return GetFlecsWorldChecked()->GetTagEntity(InTag);
}

