// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Entities/FlecsCommonHandle.h"

#include "Worlds/FlecsWorld.h"
#include "Worlds/FlecsWorldConverter.h"
#include "Worlds/UnrealFlecsWorldTag.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsCommonHandle)

FFlecsCommonHandle::FFlecsCommonHandle(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld, const FFlecsId InEntity)
{
	SetEntity(flecs::entity(InWorld->GetNativeFlecsWorld(), InEntity));
}

FFlecsCommonHandle::FFlecsCommonHandle(const flecs::world_t* InWorld, const FFlecsId InEntity)
{
	SetEntity(flecs::entity(InWorld, InEntity));
}

flecs::world FFlecsCommonHandle::GetNativeFlecsWorld() const
{
	return Entity.world();
}

UFlecsWorldInterfaceObject* FFlecsCommonHandle::GetFlecsWorld() const
{
	solid_checkf(IsUnrealFlecsWorld(), TEXT("Entity is not in an Unreal Flecs World"));
	return UE::Flecs::ToUnrealFlecsWorld(GetEntity().world());
}

TSolidNotNull<UFlecsWorldInterfaceObject*> FFlecsCommonHandle::GetFlecsWorldChecked() const
{
	UFlecsWorldInterfaceObject* FlecsWorld = GetFlecsWorld();
	
	solid_cassumef(FlecsWorld, TEXT("Flecs World not found"));
	solid_checkf(::IsValid(FlecsWorld), TEXT("Flecs World not found"));
	
	return FlecsWorld;
}

bool FFlecsCommonHandle::IsUnrealFlecsWorld() const
{
	return GetNativeFlecsWorld().has<FUnrealFlecsWorldTag>();
}

TSolidNotNull<UWorld*> FFlecsCommonHandle::GetOuterWorld() const
{
	solid_checkf(IsUnrealFlecsWorld(), TEXT("Entity is not in an Unreal Flecs World"));
	solid_checkf(::IsValid(GetFlecsWorld()), TEXT("Flecs World not found"));
	return GetFlecsWorldChecked()->GetWorld();
}

FFlecsId FFlecsCommonHandle::ObtainComponentTypeStruct(const TSolidNotNull<const UScriptStruct*> StructType) const
{
	return GetFlecsWorldChecked()->GetScriptStructEntity(StructType);
}

FFlecsId FFlecsCommonHandle::ObtainComponentTypeEnum(const TSolidNotNull<const UEnum*> EnumType) const
{
	return GetFlecsWorldChecked()->GetScriptEnumEntity(EnumType);
}

FFlecsId FFlecsCommonHandle::ObtainTypeClass(const TSolidNotNull<UClass*> ClassType) const
{
	return GetFlecsWorldChecked()->ObtainTypedEntity(ClassType);
}

FFlecsId FFlecsCommonHandle::GetTagEntity(const FGameplayTag& InTag) const
{
	return GetFlecsWorldChecked()->GetTagEntity(InTag);
}

