﻿// Elie Wiese-Namir © 2025. All Rights Reserved.

// ReSharper disable CppTooWideScopeInitStatement
#include "FlecsEntityHandle.h"
#include "Components/FlecsWorldPtrComponent.h"
#include "Networking/FlecsNetworkIdComponent.h"
#include "Worlds/FlecsWorld.h"
#include "Worlds/FlecsWorldSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsEntityHandle)

FFlecsEntityHandle FFlecsEntityHandle::GetNullHandle(const TSolidNonNullPtr<UFlecsWorld> InWorld)
{
    return flecs::entity::null(InWorld->World);
}

FFlecsEntityHandle::FFlecsEntityHandle(const TSolidNonNullPtr<UFlecsWorld> InWorld, const FFlecsId InEntity)
{
    SetEntity(flecs::entity(InWorld->World, InEntity));
}

UFlecsWorld* FFlecsEntityHandle::GetFlecsWorld() const
{
    return ToFlecsWorld(GetEntity().world());
}

UWorld* FFlecsEntityHandle::GetOuterWorld() const
{
    solid_checkf(::IsValid(GetFlecsWorld()), TEXT("Flecs World not found"));
    return GetFlecsWorld()->GetSingletonPtr<FUWorldPtrComponent>()->World.Get();
}

FString FFlecsEntityHandle::GetWorldName() const
{
    return GetFlecsWorld()->GetWorldName();
}

bool FFlecsEntityHandle::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
    return true;
}

FFlecsEntityHandle FFlecsEntityHandle::ObtainComponentTypeStruct(const UScriptStruct* StructType) const
{
    solid_checkf(::IsValid(StructType), TEXT("Struct type is not valid"));
    return GetFlecsWorld()->ObtainComponentTypeStruct(StructType);
}

FFlecsEntityHandle FFlecsEntityHandle::ObtainComponentTypeEnum(const UEnum* EnumType) const
{
    solid_checkf(::IsValid(EnumType), TEXT("Enum type is not valid"));
    return GetFlecsWorld()->ObtainComponentTypeEnum(EnumType);
}

// void FFlecsEntityHandle::AddCollection(UObject* Collection) const
// {
//     solid_check(::IsValid(Collection));
//     UFlecsComponentCollectionObject* ComponentCollection = CastChecked<UFlecsComponentCollectionObject>(Collection);
//    // ComponentCollection->ApplyCollection_Internal(*this, GetFlecsWorld());
// }

FFlecsEntityHandle FFlecsEntityHandle::GetTagEntity(const FGameplayTag& InTag) const
{
    return GetFlecsWorld()->GetTagEntity(InTag);
}

