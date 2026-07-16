// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/FlecsIrisReplicationTransport.h"

#include "Engine/NetConnection.h"
#include "Engine/NetDriver.h"
#include "Engine/World.h"

#include "Networking/FlecsIrisReplicationShard.h"
#include "Networking/FlecsNetworkingModuleSettings.h"
#include "Networking/FlecsNetworkWorldSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsIrisReplicationTransport)

bool UFlecsIrisReplicationTransport::InitializeTransport(UFlecsNetworkWorldSubsystem* InSubsystem)
{
	return Super::InitializeTransport(InSubsystem) && InSubsystem && InSubsystem->GetWorld()
		&& InSubsystem->GetWorld()->GetNetMode() != NM_Standalone;
}

void UFlecsIrisReplicationTransport::ShutdownTransport()
{
	for (const TPair<FName, TObjectPtr<UFlecsIrisReplicationShard>>& Pair : Shards)
	{
		if (Pair.Value)
		{
			Pair.Value->StopReplication();
		}
	}
	
	Shards.Reset();
	
	Super::ShutdownTransport();
}

void UFlecsIrisReplicationTransport::TickTransport()
{
	bool bAllStarted = true;
	
	for (const TPair<FName, TObjectPtr<UFlecsIrisReplicationShard>>& Pair : Shards)
	{
		if (Pair.Value)
		{
			bAllStarted &= Pair.Value->TryStartReplication();
		}
	}
	
	if (!Shards.IsEmpty() && !bAllStarted && ++StartAttempts > 120 && !bWarnedReplicationSystemUnavailable)
	{
		bWarnedReplicationSystemUnavailable = true;
		UE_LOG(LogFlecsCore, Warning,
			TEXT("UnrealFlecs Iris shards could not attach to a compatible ReplicationSystem. Enable Iris on the active NetDriver; Flecs simulation remains active without network transport."));
	}
}

void UFlecsIrisReplicationTransport::PublishLayout(const FFlecsReplicationRouteKey& Route,
	const FFlecsReplicationLayoutDefinition& Layout)
{
	if (UFlecsIrisReplicationShard* Shard = FindOrCreateShard(Route))
	{
		Shard->UpsertLayout(Layout);
	}
}

void UFlecsIrisReplicationTransport::PublishEntity(const FFlecsReplicationRouteKey& Route,
	const FFlecsReplicatedEntitySnapshot& Snapshot)
{
	if (UFlecsIrisReplicationShard* Shard = FindOrCreateShard(Route))
	{
		Shard->UpsertEntity(Snapshot);
	}
}

void UFlecsIrisReplicationTransport::RemoveEntity(const FFlecsReplicationRouteKey& Route,
	const FFlecsNetworkId NetworkId)
{
	TObjectPtr<UFlecsIrisReplicationShard>* Shard = Shards.Find(Route.Name);
	if (Shard && *Shard)
	{
		(*Shard)->RemoveEntity(NetworkId);
	}
}

void UFlecsIrisReplicationTransport::HandleProtocolError(const FString& Diagnostic)
{
	Super::HandleProtocolError(Diagnostic);
	
	if (const UFlecsNetworkWorldSubsystem* Subsystem = GetNetworkSubsystem())
	{
		const UNetDriver* Driver = Subsystem->GetWorld()->GetNetDriver(); 
		
		if (Driver && Driver->ServerConnection)
		{
			Driver->ServerConnection->Close();
		}
	}
}

UFlecsIrisReplicationShard* UFlecsIrisReplicationTransport::FindOrCreateShard(
	const FFlecsReplicationRouteKey& Route)
{
	if (TObjectPtr<UFlecsIrisReplicationShard>* Found = Shards.Find(Route.Name))
	{
		return *Found;
	}
	
	UFlecsNetworkWorldSubsystem* Subsystem = GetNetworkSubsystem();
	if (!Subsystem || !Subsystem->HasAuthority())
	{
		return nullptr;
	}
	
	const TSolidNotNull<UFlecsIrisReplicationShard*> Shard = NewObject<UFlecsIrisReplicationShard>(Subsystem);
	const TSolidNotNull<const UFlecsNetworkingModuleSettings*> Settings = GetDefault<UFlecsNetworkingModuleSettings>();
	
	Shard->InitializeServer(Subsystem->GetWorld(), Route, Settings->DefaultShardPollFrequency,
		Settings->DefaultShardStaticPriority);
	Shards.Add(Route.Name, Shard);
	
	return Shard;
}
