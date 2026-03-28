// Elie Wiese-Namir © 2025. All Rights Reserved.

// ReSharper disable CppTooWideScopeInitStatement
#include "Entities/FlecsEntityHandle.h"

#include "Collections/FlecsCollectionWorldSubsystem.h"
#include "Logging/StructuredLog.h"

#include "Logs/FlecsCategories.h"

#include "Worlds/FlecsWorld.h"
#include "Worlds/FlecsWorldSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsEntityHandle)

FFlecsEntityHandle FFlecsEntityHandle::GetNullHandle(const TSolidNotNull<const UFlecsWorld*> InWorld)
{
    return flecs::entity::null(InWorld->World);
}

const FFlecsEntityHandle::FSelfType& FFlecsEntityHandle::AddCollection(const FFlecsId InCollection,
    const FInstancedStruct& InParams) const
{
    solid_checkf(InCollection.IsValid(),
        TEXT("Trying to add an invalid collection to an entity!"));
    solid_checkf(GetNativeFlecsWorld().has<FFlecsCollectionSubsystemSingleton>(),
        TEXT("Trying to add a collection to an entity, but the FlecsCollectionSubsystemSingleton is not registered in the world!"));
    
    const FFlecsCollectionSubsystemSingleton& CollectionSubsystemSingleton
       = GetNativeFlecsWorld().get<FFlecsCollectionSubsystemSingleton>();

    solid_checkf(CollectionSubsystemSingleton.WorldSubsystem.IsValid(),
        TEXT("Trying to add a collection to an entity, but the FlecsCollectionWorldSubsystem is not initialized!"));

    CollectionSubsystemSingleton.WorldSubsystem->AddCollectionToEntity(*this, InCollection, InParams);
    return *this;
}

const FFlecsEntityHandle::FSelfType& FFlecsEntityHandle::AddCollection(const FFlecsCollectionReference& InCollectionRef,
    const FInstancedStruct& InParams) const
{
    solid_checkf(GetNativeFlecsWorld().has<FFlecsCollectionSubsystemSingleton>(),
        TEXT("Trying to add a collection to an entity, but the FlecsCollectionSubsystemSingleton is not registered in the world!"));
    
    const FFlecsCollectionSubsystemSingleton& CollectionSubsystemSingleton
       = GetNativeFlecsWorld().get<FFlecsCollectionSubsystemSingleton>();

    solid_checkf(CollectionSubsystemSingleton.WorldSubsystem.IsValid(),
        TEXT("Trying to add a collection to an entity, but the FlecsCollectionWorldSubsystem is not initialized!"));

    CollectionSubsystemSingleton.WorldSubsystem->AddCollectionToEntity(*this, InCollectionRef, InParams);
    return *this;
}

const FFlecsEntityHandle::FSelfType& FFlecsEntityHandle::AddCollection(
    const FFlecsCollectionInstancedReference& InCollectionRef) const
{
    return AddCollection(InCollectionRef.Collection, InCollectionRef.Parameters);
}

const FFlecsEntityHandle::FSelfType& FFlecsEntityHandle::RemoveCollection(const FFlecsId InCollection) const
{
    solid_checkf(InCollection.IsValid(),
      TEXT("Trying to add an invalid collection to an entity!"));
    solid_checkf(GetNativeFlecsWorld().has<FFlecsCollectionSubsystemSingleton>(),
        TEXT("Trying to add a collection to an entity, but the FlecsCollectionSubsystemSingleton is not registered in the world!"));

    const FFlecsCollectionSubsystemSingleton& CollectionSubsystemSingleton
       = GetNativeFlecsWorld().get<FFlecsCollectionSubsystemSingleton>();

    solid_checkf(CollectionSubsystemSingleton.WorldSubsystem.IsValid(),
        TEXT("Trying to add a collection to an entity, but the FlecsCollectionWorldSubsystem is not initialized!"));

    CollectionSubsystemSingleton.WorldSubsystem->RemoveCollectionFromEntity(*this, InCollection);
    return *this;
}
