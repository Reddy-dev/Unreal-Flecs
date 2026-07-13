// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "Networking/FlecsReplicationTransportBase.h"

#include "FlecsIrisReplicationTransport.generated.h"

class UFlecsIrisReplicationShard;

/**
 * Iris implementation of the Flecs transport boundary.
 *
 * Each route is represented by one aggregate UFlecsIrisReplicationShard,
 * rather than one replicated UObject per Flecs entity. The core still owns
 * schema validation and all Flecs mutations.
 */
UCLASS(Transient)
class UNREALFLECSIRIS_API UFlecsIrisReplicationTransport : public UFlecsReplicationTransportBase
{
	GENERATED_BODY()

public:
	/** Valid only for a non-standalone world with a usable NetDriver. */
	virtual bool InitializeTransport(UFlecsNetworkWorldSubsystem* InSubsystem) override;
	virtual void ShutdownTransport() override;
	virtual void TickTransport() override;
	virtual void PublishLayout(const FFlecsReplicationRouteKey& Route,
		const FFlecsReplicationLayoutDefinition& Layout) override;
	virtual void PublishEntity(const FFlecsReplicationRouteKey& Route,
		const FFlecsReplicatedEntitySnapshot& Snapshot) override;
	virtual void RemoveEntity(const FFlecsReplicationRouteKey& Route, FFlecsNetworkId NetworkId) override;
	virtual void HandleProtocolError(const FString& Diagnostic) override;

private:
	UFlecsIrisReplicationShard* FindOrCreateShard(const FFlecsReplicationRouteKey& Route);

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UFlecsIrisReplicationShard>> Shards;

	uint32 StartAttempts = 0;
	bool bWarnedReplicationSystemUnavailable = false;
};
