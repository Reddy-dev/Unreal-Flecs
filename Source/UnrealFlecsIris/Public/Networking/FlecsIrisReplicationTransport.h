// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "Networking/FlecsReplicationTransportBase.h"

#include "FlecsIrisReplicationTransport.generated.h"

class UFlecsIrisReplicationShard;

/** Bounded, filtered, page-based Iris adapter for Flecs replication updates. */
UCLASS(Transient)
class UNREALFLECSIRIS_API UFlecsIrisReplicationTransport : public UFlecsReplicationTransportBase
{
	GENERATED_BODY()

public:
	virtual bool InitializeTransport(UFlecsNetworkWorldSubsystem* InSubsystem) override;
	virtual void ShutdownTransport() override;
	virtual void TickTransport() override;
	virtual void PublishLayout(const FFlecsReplicationRouteDescriptor& Route,
		const FFlecsReplicationLayoutDefinition& Layout) override;
	virtual void PublishEntity(const FFlecsReplicationRouteDescriptor& Route,
		const FFlecsReplicatedEntityUpdate& Update) override;
	virtual void MigrateEntity(const FFlecsReplicationRouteDescriptor& OldRoute,
		const FFlecsReplicationRouteDescriptor& NewRoute, const FFlecsReplicationLayoutDefinition& Layout,
		const FFlecsReplicatedEntityUpdate& FullUpdate) override;
	virtual void RemoveEntity(const FFlecsReplicationRouteDescriptor& Route, FFlecsNetworkId NetworkId) override;
	virtual void SetEntityDormancy(const FFlecsReplicationRouteDescriptor& Route,
		FFlecsNetworkId NetworkId, bool bDormant) override;
	virtual void HandleProtocolError(const FString& Diagnostic) override;

	NO_DISCARD int32 GetPageCount(FName LogicalRoute = NAME_None) const;
	NO_DISCARD UFlecsIrisReplicationShard* FindEntityPage(FFlecsNetworkId NetworkId) const;

private:
	void EnsureReplicationFilter();
	FFlecsReplicatedEntityUpdate MaterializeUpdate(const FFlecsReplicatedEntityUpdate& Update) const;
	UFlecsIrisReplicationShard* FindPageWithCapacity(const FFlecsReplicationRouteDescriptor& Route,
		FFlecsNetworkId NetworkId, uint32 PayloadBytes, FName ExcludedPage = NAME_None);
	UFlecsIrisReplicationShard* CreatePage(const FFlecsReplicationRouteDescriptor& Route);
	void QueueRetirementIfEmpty(FName PageName);
	void RetirePage(FName PageName);
	NO_DISCARD FName FindPageName(const UFlecsIrisReplicationShard* Page) const;

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UFlecsIrisReplicationShard>> Pages;

	TMap<FName, TArray<FName>> RoutePageNames;
	TMap<FName, uint32> NextPageIndices;
	TMap<FFlecsNetworkId, FName> EntityPageNames;
	TMap<FFlecsNetworkId, FFlecsReplicatedEntityUpdate> MaterializedEntities;
	TMap<FName, TMap<FFlecsReplicationLayoutId, FFlecsReplicationLayoutDefinition>> RouteLayouts;
	TMap<FName, uint8> PendingRetirements;
	uint32 StartAttempts = 0;
	bool bWarnedReplicationSystemUnavailable = false;
	bool bCreatedRuntimeFilter = false;
}; // class UFlecsIrisReplicationTransport
