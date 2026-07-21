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
	virtual void PublishLayout(const FFlecsReplicationRouteDescriptor& Route,
		const FFlecsReplicationLayoutDefinition& Layout) override;
	virtual void PublishEntity(const FFlecsReplicationRouteDescriptor& Route,
		const FFlecsReplicatedEntityUpdate& Update) override;
	virtual void MigrateEntity(const FFlecsReplicationRouteDescriptor& OldRoute,
		const FFlecsReplicationRouteDescriptor& NewRoute, const FFlecsReplicationLayoutDefinition& Layout,
		const FFlecsReplicatedEntityUpdate& FullUpdate) override;
	virtual void RemoveEntity(const FFlecsReplicationRouteDescriptor& Route, FFlecsNetworkId NetworkId) override;
	virtual void HandleProtocolError(const FString& Diagnostic) override;

#if WITH_AUTOMATION_TESTS || WITH_EDITOR
	NO_DISCARD int32 GetPageCountForTesting() const
	{
		return Pages.Num();
	}

	NO_DISCARD const UFlecsIrisReplicationShard* GetEntityPageForTesting(FFlecsNetworkId NetworkId) const
	{
		return EntityPages.FindRef(NetworkId);
	}
#endif

private:
	void EnsureReplicationFilter();
	UFlecsIrisReplicationShard* FindPageForUpdate(const FFlecsReplicationRouteDescriptor& Route,
		const FFlecsReplicatedEntityUpdate& Update, UFlecsIrisReplicationShard* ExcludedPage = nullptr);
	UFlecsIrisReplicationShard* CreatePage(const FFlecsReplicationRouteDescriptor& Route);
	void PublishToPage(UFlecsIrisReplicationShard& Page, const FFlecsReplicatedEntityUpdate& Update);
	void ScheduleRetirementIfEmpty(UFlecsIrisReplicationShard& Page);
	NO_DISCARD FFlecsReplicatedEntityUpdate MaterializeUpdate(const FFlecsReplicatedEntityUpdate& Update) const;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UFlecsIrisReplicationShard>> Pages;

	UPROPERTY(Transient)
	TMap<FFlecsNetworkId, TObjectPtr<UFlecsIrisReplicationShard>> EntityPages;

	UPROPERTY()
	TMap<FFlecsReplicationLayoutId, FFlecsReplicationLayoutDefinition> Layouts;
	
	UPROPERTY()
	TMap<FFlecsNetworkId, FFlecsReplicatedEntityUpdate> MaterializedEntities;
	
	UPROPERTY()
	TMap<TObjectPtr<UFlecsIrisReplicationShard>, uint8> PendingRetirementTicks;

	UPROPERTY()
	uint32 StartAttempts = 0;
	
	UPROPERTY()
	bool bWarnedReplicationSystemUnavailable = false;
	
	UPROPERTY()
	bool bCreatedRuntimeFilter = false;
};
