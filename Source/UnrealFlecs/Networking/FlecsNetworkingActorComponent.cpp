﻿// Fill out your copyright notice in the Description page of Project Settings.

#include "FlecsNetworkingActorComponent.h"
#include "Logs/FlecsCategories.h"
#include "Worlds/FlecsWorldSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsNetworkingActorComponent)

void UFlecsNetworkingActorComponent::Client_UpdateCreatedNetworkedEntities_Implementation(
	const TArray<FNetworkedEntityInfo>& Entities)
{
	for (const auto& [NetworkId, WorldName, EntityName] : Entities)
	{
		solid_checkf(!EntityName.IsEmpty(),TEXT("Networked Entity Name must not be None"));

		const UFlecsWorld* FlecsWorld = GetWorld()->GetSubsystem<UFlecsWorldSubsystem>()->GetDefaultWorld();
		
		if UNLIKELY_IF(!ensureMsgf(FlecsWorld, TEXT("FlecsWorld %s not found"), *WorldName))
		{
			continue;
		}
		
		FFlecsEntityHandle Entity = FlecsWorld->LookupEntity(EntityName);
		
		if UNLIKELY_IF(!Entity.IsValid())
		{
			UN_LOGF(LogFlecsNetworking, Warning,
				"Entity %s not found in world %s",
				*EntityName,
				*WorldName);
			
			Entity = FlecsWorld->CreateEntity(EntityName);

			UN_LOGF(LogFlecsNetworking, Log,
				"Created entity %s in world %s, sent from server",
				*EntityName,
				*WorldName);
		}

		Entity.Set<FFlecsNetworkIdComponent>(NetworkId);

		UN_LOGF(LogFlecsNetworking, Log,
			"Added network ID %llu to entity %s in world %s",
			NetworkId.GetNetworkId(),
			*EntityName,
			*WorldName);
	}
}
