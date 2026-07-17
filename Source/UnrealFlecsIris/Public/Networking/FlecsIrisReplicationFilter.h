// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Iris/ReplicationSystem/Filtering/NetObjectFilter.h"

#include "FlecsIrisReplicationFilter.generated.h"

class UFlecsIrisReplicationShard;
class UReplicationSystem;

/** Dynamic Iris filter that delegates page relevance to the Flecs interest policy. */
UCLASS(Transient)
class UNREALFLECSIRIS_API UFlecsIrisReplicationFilter final : public UNetObjectFilter
{
	GENERATED_BODY()

public:
	static FName GetFilterName();
	void RegisterPage(UE::Net::FNetRefHandle Handle, UFlecsIrisReplicationShard* Page);

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
	TMap<UE::Net::FInternalNetRefIndex, TWeakObjectPtr<UFlecsIrisReplicationShard>> Pages;
}; // class UFlecsIrisReplicationFilter
