﻿// Elie Wiese-Namir © 2025. All Rights Reserved.

// ReSharper disable CppTooWideScopeInitStatement
#include "FlecsEntityHandle.h"
#include "Collections/FlecsComponentCollectionObject.h"
#include "Components/FlecsWorldPtrComponent.h"
#include "Worlds/FlecsWorld.h"
#include "Worlds/FlecsWorldSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsEntityHandle)

DEFINE_FLECS_ENTITY_NET_SERIALIZE_FUNCTION(Unreal::Flecs::EmptyNetSerializeFunction, [](
    FFlecsEntityHandle& InEntity, TSolidNotNull<UFlecsWorld*> FlecsWorld, FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
    if UNLIKELY_IF(!InEntity.IsValid())
    {
        UE_LOGFMT(LogFlecsNetworking, Warning,
            "Trying to net serialize an invalid entity");
        bOutSuccess = true;
        return true;
    }
    
    if (InEntity.HasName())
    {
        UE_LOGFMT(LogFlecsNetworking, Warning,
            "Trying to net serialize entity {EntityId} with a name ({EntityName}) without a valid NetSerialize function",
            InEntity.GetFlecsId().ToString(), InEntity.GetName());
    }
    else
    {
        UE_LOGFMT(LogFlecsNetworking, Warning,
        "Trying to net serialize entity {EntityId} without a valid NetSerialize function",
                 InEntity.GetFlecsId().ToString());
    }
        
    bOutSuccess = true;
    return true;
});

Unreal::Flecs::FEntityNetSerializeFunction* Unreal::Flecs::GNetSerializeFunctionPtr = &EmptyNetSerializeFunction; 

FFlecsEntityHandle FFlecsEntityHandle::GetNullHandle(const TSolidNotNull<const UFlecsWorld*> InWorld)
{
    return flecs::entity::null(InWorld->World);
}

FFlecsEntityHandle::FFlecsEntityHandle(const TSolidNotNull<const UFlecsWorld*> InWorld, const FFlecsId InEntity)
{
    SetEntity(flecs::entity(InWorld->World, InEntity));
}

FFlecsEntityHandle::FFlecsEntityHandle(const flecs::world_t* InWorld, const FFlecsId InEntity)
{
    SetEntity(flecs::entity(InWorld, InEntity));
}

TSolidNotNull<UFlecsWorld*> FFlecsEntityHandle::GetFlecsWorld() const
{
    return Unreal::Flecs::ToFlecsWorld(GetEntity().world());
}

TSolidNotNull<UWorld*> FFlecsEntityHandle::GetOuterWorld() const
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
    if LIKELY_IF(Unreal::Flecs::GNetSerializeFunctionPtr)
    {
        return std::invoke(
            *Unreal::Flecs::GNetSerializeFunctionPtr, *this, GetFlecsWorld(), Ar, Map, bOutSuccess);
    }
    else
    {
        UE_LOG(LogFlecsNetworking, Error,
            TEXT("Trying to net serialize entity with a nullptr NetSerialize function ptr"));
        
        bOutSuccess = false;
        return false;
    }
    
}

FFlecsEntityHandle FFlecsEntityHandle::ObtainComponentTypeStruct(const TSolidNotNull<const UScriptStruct*> StructType) const
{
    return GetFlecsWorld()->ObtainComponentTypeStruct(StructType);
}

FFlecsEntityHandle FFlecsEntityHandle::ObtainComponentTypeEnum(const TSolidNotNull<const UEnum*> EnumType) const
{
    return GetFlecsWorld()->ObtainComponentTypeEnum(EnumType);
}

void FFlecsEntityHandle::AddCollection(const TSolidNotNull<UObject*> Collection) const
{
    const TSolidNotNull<UFlecsComponentCollectionObject*> ComponentCollection
        = CastChecked<UFlecsComponentCollectionObject>(Collection);
    
    ComponentCollection->ApplyCollection_Internal(*this, GetFlecsWorld());
}

FFlecsEntityHandle FFlecsEntityHandle::GetTagEntity(const FGameplayTag& InTag) const
{
    solid_checkf(InTag.IsValid(), TEXT("Invalid GameplayTag provided for GetTagEntity"));
    return GetFlecsWorld()->GetTagEntity(InTag);
}

