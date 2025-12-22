// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "GameFramework/Actor/FlecsEntityActorComponent.h"

#include "Engine/World.h"

#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

#include "Versioning/SolidVersioningTypes.h"

#include "Logs/FlecsCategories.h"

#include "Worlds/FlecsWorld.h"
#include "Worlds/FlecsWorldSubsystem.h"

#include "Components/FlecsUObjectComponent.h"
#include "Components/ObjectTypes/FlecsActorTag.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsEntityActorComponent)

IMPLEMENT_SOLID_ASSET_VERSION(UFlecsEntityActorComponent, 0xA9464252, 0x37464046, 0xBDFF76B4, 0xEBBDA15A, "FlecsEntityActorComponentVersion");

REGISTER_ASSET_MIGRATION_STEP(UFlecsEntityActorComponent, SwitchedEntityInitializationMethodToEntityInitializationType,
	{
		const bool bChanged = !(Self->EntityRecord_DEPRECATED == Archetype->EntityRecord_DEPRECATED);
		if (bChanged)
		{
			Self->EntityInitializer.Type = EFlecsEntityInitializationType::Record;
			Self->EntityInitializer.Record = Self->EntityRecord_DEPRECATED;
		}
	})

UFlecsEntityActorComponent::UFlecsEntityActorComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	//SetIsReplicatedByDefault(true);
}

void UFlecsEntityActorComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UFlecsEntityActorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DestroyEntity();
	
	Super::EndPlay(EndPlayReason);
}

void UFlecsEntityActorComponent::PostLoad()
{
	Super::PostLoad();

	IMPLEMENT_ASSET_MIGRATION_POST_LOAD(UFlecsEntityActorComponent);
}

void UFlecsEntityActorComponent::OnRegister()
{
	Super::OnRegister();

	InitializeEntity();
}

void UFlecsEntityActorComponent::OnUnregister()
{
		
	Super::OnUnregister();
}

void UFlecsEntityActorComponent::InitializeEntity()
{
	if UNLIKELY_IF(!GWorld  || !GWorld->IsGameWorld())
	{
		return;
	}

	if UNLIKELY_IF(EntityHandle.GetEntity().id() != 0)
	{
		return;
	}

	if LIKELY_IF(const UFlecsWorld* World = UFlecsWorldSubsystem::GetDefaultWorldStatic(this))
	{
		CreateActorEntity(World);
	}
	else
	{
		UE_LOGFMT(LogFlecsEntity, Error,
			"Could not initialize Entity for Actor Component on Actor: {ActorName} - No valid Flecs World found.",
			GetOwner()->GetName());
	}
}

void UFlecsEntityActorComponent::DestroyEntity(const bool bDestroyOwningActor)
{
	AActor* OwnerActor = GetOwner();
	
	if UNLIKELY_IF(!OwnerActor)
	{
		UE_LOGFMT(LogFlecsEntity, Warning,
			"Attempted to destroy Entity from Actor Component with no valid Owner Actor.");
		return;
	}
		
	if LIKELY_IF(EntityHandle.IsValid())
	{
		EntityHandle.Destroy();
		EntityHandle = FFlecsEntityHandle::GetNullHandle();
	}
	else
	{
		UE_LOGFMT(LogFlecsEntity, Warning,
			"Attempted to destroy invalid Entity from Actor Component on Actor: {ActorName}",
			GetOwner()->GetName());
	}

	if (bDestroyOwningActor)
	{
		OwnerActor->Destroy();
	}
}

void UFlecsEntityActorComponent::OnEntitySpawned(const FFlecsEntityHandle& InEntityHandle)
{
	BP_OnEntitySpawned(InEntityHandle);
}

void UFlecsEntityActorComponent::SetEntityHandle(const FFlecsEntityHandle& InEntityHandle)
{
	unimplemented();
	//EntityHandle = InEntityHandle;
	//MARK_PROPERTY_DIRTY_FROM_NAME(UFlecsEntityActorComponent, EntityHandle, this);
}

void UFlecsEntityActorComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(UFlecsEntityActorComponent, EntityHandle, Params);
}

#if WITH_EDITOR

bool UFlecsEntityActorComponent::CanEditChange(const FProperty* InProperty) const
{
	const bool bIsEditable = Super::CanEditChange(InProperty);
	
	return bIsEditable;
}

#endif // WITH_EDITOR

// Currently unused, but may be useful in the future
void UFlecsEntityActorComponent::OnWorldCreated(UFlecsWorld* InWorld)
{
	CreateActorEntity(InWorld);
	
	GetWorld()->GetSubsystem<UFlecsWorldSubsystem>()->OnWorldCreatedDelegate.RemoveAll(this);
}

void UFlecsEntityActorComponent::CreateActorEntity(const TSolidNotNull<const UFlecsWorld*> InWorld)
{
	EntityHandle = InitializeComponentEntity(InWorld);
	EntityHandle.SetName(GetOwner()->GetName());
	
	// @TODO: Should this be the Component or the Owner?
	EntityHandle.SetPair<FFlecsUObjectComponent, FFlecsActorTag>(FFlecsUObjectComponent(GetOwner()));

	if (EntityHandle.HasName())
	{
		UE_LOGFMT(LogFlecsEntity, Log,
			"Creating Actor Entity with name: {EntityName}", EntityHandle.GetName());
	}
	else
	{
		UE_LOGFMT(LogFlecsEntity, Log,
			"Creating Actor Entity with id: {EntityId}", EntityHandle.GetFlecsId().ToString());
	}

	//MARK_PROPERTY_DIRTY_FROM_NAME(UFlecsEntityActorComponent, EntityHandle, this);

	OnEntitySpawned(EntityHandle);
}

FFlecsEntityHandle UFlecsEntityActorComponent::InitializeComponentEntity(
	const TSolidNotNull<const UFlecsWorld*> InWorld) const
{
	FFlecsEntityHandle CreatedEntity;
	switch (EntityInitializer.Type)
	{
		case EFlecsEntityInitializationType::Record:
			{
				CreatedEntity = InitializeUsingEntityRecord(InWorld, EntityInitializer.Record);
				
				break;
			}
		case EFlecsEntityInitializationType::InstancedCollection:
			{
				CreatedEntity = InitializeUsingCollection(InWorld, EntityInitializer.InstancedCollection);
				
				break;
			}
		default: UNLIKELY_ATTRIBUTE
			{
				solid_checkf(false, TEXT("UFlecsEntityActorComponent::InitializeComponentEntity: Unknown Entity Initialization Type"));
				break;
			}
	}

	solid_checkf(CreatedEntity.IsValid(),
		TEXT("UFlecsEntityActorComponent::InitializeComponentEntity: Created Entity is not valid"));
	return CreatedEntity;
}

FFlecsEntityHandle UFlecsEntityActorComponent::InitializeUsingEntityRecord(
	const TSolidNotNull<const UFlecsWorld*> InWorld, const FFlecsEntityRecord& InRecord) const
{
	const FFlecsEntityHandle NewEntity = InWorld->CreateEntityWithRecord(InRecord);
	return NewEntity;
}

FFlecsEntityHandle UFlecsEntityActorComponent::InitializeUsingCollection(const TSolidNotNull<const UFlecsWorld*> InWorld,
	const FFlecsCollectionInstancedReference& InCollectionRef) const
{
	const FFlecsEntityHandle NewEntity = InWorld->CreateEntity();
	NewEntity.AddCollection(InCollectionRef);
	return NewEntity;
}
