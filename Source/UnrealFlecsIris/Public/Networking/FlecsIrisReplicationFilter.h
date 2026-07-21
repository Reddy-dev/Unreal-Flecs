// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Iris/ReplicationSystem/Filtering/NetObjectFilter.h"
#include "Networking/FlecsReplicationTypes.h"

#include "FlecsIrisReplicationFilter.generated.h"

class APlayerController;
class UFlecsIrisReplicationShard;
class UFlecsNetworkWorldSubsystem;
class UNetConnection;
class UReplicationSystem;

/** Resolves Iris's transport parent connection ID without exposing Iris types to the core. */
UNREALFLECSIRIS_API FFlecsReplicationConnectionId GetFlecsReplicationConnectionId(
	const UNetConnection* Connection);
UNREALFLECSIRIS_API FFlecsReplicationConnectionId GetFlecsReplicationConnectionId(
	const APlayerController* Controller);

/** Dynamic Iris filter that delegates page relevance to the Flecs policy registry. */
UCLASS(Transient)
class UNREALFLECSIRIS_API UFlecsIrisReplicationFilter final : public UNetObjectFilter
{
	GENERATED_BODY()

public:
	static FName GetFilterName();
	
	void RegisterPage(UE::Net::FNetRefHandle Handle, UFlecsIrisReplicationShard* Page);
	
	virtual void AddConnection(uint32 ConnectionId) override;
	virtual void RemoveConnection(uint32 ConnectionId) override;

protected:
	virtual void OnInit(const FNetObjectFilterInitParams& Params) override;
	virtual void OnDeinit() override;
	
	virtual void OnMaxInternalNetRefIndexIncreased(UE::Net::FInternalNetRefIndex NewMaxInternalIndex) override;
	
	virtual bool AddObject(UE::Net::FInternalNetRefIndex ObjectIndex,
		FNetObjectFilterAddObjectParams& Params) override;
	virtual void RemoveObject(UE::Net::FInternalNetRefIndex ObjectIndex,
		const FNetObjectFilteringInfo& Info) override;
	
	virtual void Filter(FNetObjectFilteringParams& Params) override;

private:
	TWeakObjectPtr<UReplicationSystem> ReplicationSystem;
	TWeakObjectPtr<UFlecsNetworkWorldSubsystem> NetworkSubsystem;
	TMap<UE::Net::FInternalNetRefIndex, TWeakObjectPtr<UFlecsIrisReplicationShard>> Pages;
	TSet<uint32> Connections;
}; // class UFlecsIrisReplicationFilter
