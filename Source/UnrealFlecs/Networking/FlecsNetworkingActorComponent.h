﻿// Fill out your copyright notice in the Description page of Project Settings.

// ReSharper disable CppMemberFunctionMayBeStatic
#pragma once

#include "CoreMinimal.h"
#include "FlecsNetworkIdComponent.h"
#include "FlecsNetworkingManager.h"
#include "Components/ActorComponent.h"
#include "Entities/FlecsEntityHandle.h"
#include "GameFramework/GameState.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "Worlds/FlecsWorldSubsystem.h"
#include "FlecsNetworkingActorComponent.generated.h"

USTRUCT()
struct FNetworkedEntityInfo
{
	GENERATED_BODY()
	
	UPROPERTY()
	FFlecsNetworkIdComponent NetworkId;

	UPROPERTY()
	FString WorldName = TEXT("DefaultFlecsWorld");

	UPROPERTY()
	FString EntityName;

	FORCEINLINE bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
	{
		NetworkId.NetSerialize(Ar, Map, bOutSuccess);
		SerializeOptionalValue<FString>(Ar.IsSaving(), Ar, WorldName, TEXT("DefaultFlecsWorld"));
		Ar << EntityName;

		bOutSuccess = true;
		return true;
	}
	
}; // struct FNetworkedEntityInfo

template<>
struct TStructOpsTypeTraits<FNetworkedEntityInfo> : public TStructOpsTypeTraitsBase2<FNetworkedEntityInfo>
{
	enum
	{
		WithNetSerializer = true,
	}; // enum
	
}; // struct TStructOpsTypeTraits<FNetworkedEntityInfo>

UCLASS(BlueprintType, ClassGroup=(Flecs), meta=(BlueprintSpawnableComponent))
class UNREALFLECS_API UFlecsNetworkingActorComponent final : public UActorComponent
{
	GENERATED_BODY()

public:
	UFlecsNetworkingActorComponent(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{
		SetIsReplicatedByDefault(true);
	}
	
	FORCEINLINE virtual void BeginPlay() override
	{
		Super::BeginPlay();

		solid_checkf(GetOwner()->IsA<APlayerController>(),
			TEXT("Owner of UFlecsNetworkingActorComponent must be a APlayerController"));

		const AGameStateBase* GameState = GetWorld()->GetGameState();
		solid_checkf(IsValid(GameState), TEXT("GameState must be valid"));

		UFlecsNetworkingManager* NetworkingManager = GameState->FindComponentByClass<UFlecsNetworkingManager>();
		solid_checkf(IsValid(NetworkingManager), TEXT("NetworkingManager must be valid"));

		NetworkingManager->AddNetworkingActorComponent(this);

		#if WITH_SERVER_CODE

		if (GetOwner()->HasAuthority())
		{
			const UFlecsWorld* FlecsWorld = UFlecsWorldSubsystem::GetDefaultWorldStatic(GetOwner());
			solid_checkf(FlecsWorld, TEXT("FlecsWorld must be valid"));

			TArray<FNetworkedEntityInfo> Entities;
			FlecsWorld->World.query_builder<FFlecsNetworkIdComponent>()
				.with_name_component()
				.read<FFlecsNetworkIdComponent>()
				.build()
				.each([&](const FFlecsEntityHandle& Entity, const FFlecsNetworkIdComponent& NetworkId)
				{
					Entities.Add(FNetworkedEntityInfo{ NetworkId,
						FlecsWorld->GetWorldName(), Entity.GetName() });
				});

			Client_UpdateCreatedNetworkedEntities(Entities);
		}

		#endif // WITH_SERVER_CODE
	}

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override
	{
		if LIKELY_IF(const AGameStateBase* GameState = GetWorld()->GetGameState())
		{
			if LIKELY_IF(UFlecsNetworkingManager* NetworkingManager
				= GameState->FindComponentByClass<UFlecsNetworkingManager>())
			{
				NetworkingManager->RemoveNetworkingActorComponent(this);
			}
		}

		Super::EndPlay(EndPlayReason);
	}

	FORCEINLINE virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);
		
		FDoRepLifetimeParams Params;
		Params.bIsPushBased = true;

		DOREPLIFETIME_WITH_PARAMS_FAST(UFlecsNetworkingActorComponent, NetworkOwnedEntities, Params);
	}

	UFUNCTION(BlueprintCallable, Category = "Flecs | Networking")
	FORCEINLINE void AddNetworkOwnedEntity(const FFlecsNetworkIdComponent& Entity)
	{
		checkf(Entity.IsValid(), TEXT("Entity must be valid"));
		NetworkOwnedEntities.Add(Entity);
		SortNetworkOwnedEntities();

		MARK_PROPERTY_DIRTY_FROM_NAME(UFlecsNetworkingActorComponent, NetworkOwnedEntities, this);
	}

	UFUNCTION(BlueprintCallable, Category = "Flecs | Networking")
	FORCEINLINE void RemoveNetworkOwnedEntity(const FFlecsNetworkIdComponent& Entity)
	{
		NetworkOwnedEntities.Remove(Entity);
		SortNetworkOwnedEntities();

		MARK_PROPERTY_DIRTY_FROM_NAME(UFlecsNetworkingActorComponent, NetworkOwnedEntities, this);
	}

	UFUNCTION(BlueprintCallable, Category = "Flecs | Networking")
	FORCEINLINE bool HasNetworkOwnedEntity(const FFlecsNetworkIdComponent& Entity) const
	{
		return GetNetworkOwnedEntities().Contains(Entity);
	}

	UFUNCTION(BlueprintCallable, Category = "Flecs | Networking")
	FORCEINLINE void ClearNetworkOwnedEntities()
	{
		NetworkOwnedEntities.Empty();
		
		MARK_PROPERTY_DIRTY_FROM_NAME(UFlecsNetworkingActorComponent, NetworkOwnedEntities, this);
	}

	UFUNCTION(BlueprintCallable, Category = "Flecs | Networking")
	FORCEINLINE TSet<FFlecsNetworkIdComponent> GetNetworkOwnedEntities() const
	{
		return static_cast<TSet<FFlecsNetworkIdComponent>>(NetworkOwnedEntities);
	}

private:
	UPROPERTY(Replicated)
	TArray<FFlecsNetworkIdComponent> NetworkOwnedEntities;

	UFUNCTION(Client, Reliable)
	void Client_UpdateCreatedNetworkedEntities(const TArray<FNetworkedEntityInfo>& Entities);

	FORCEINLINE void SortNetworkOwnedEntities()
	{
		NetworkOwnedEntities.Sort([](const FFlecsNetworkIdComponent& A, const FFlecsNetworkIdComponent& B)
		{
			return A.GetNetworkId() < B.GetNetworkId();
		});
	}

}; // class UFlecsNetworkingActorComponent
