// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Networking/FlecsReplicationTypes.h"

#include "FlecsReplicationTransportBase.generated.h"

class UFlecsNetworkWorldSubsystem;

class UNREALFLECS_API IFlecsReplicationRouter
{
public:
	virtual ~IFlecsReplicationRouter() = default;
	virtual FFlecsReplicationRouteKey Route(const FFlecsEntityHandle& Entity) const = 0;
};

class UNREALFLECS_API FFlecsDefaultReplicationRouter final : public IFlecsReplicationRouter
{
public:
	virtual FFlecsReplicationRouteKey Route(const FFlecsEntityHandle&) const override
	{
		return FFlecsReplicationRouteKey::Default();
	}
};

UCLASS(Abstract, Transient)
class UNREALFLECS_API UFlecsReplicationTransportBase : public UObject
{
	GENERATED_BODY()

public:
	virtual bool InitializeTransport(UFlecsNetworkWorldSubsystem* InSubsystem);
	virtual void ShutdownTransport();
	virtual void TickTransport() {}
	virtual void PublishLayout(const FFlecsReplicationRouteKey& Route,
		const FFlecsReplicationLayoutDefinition& Layout) PURE_VIRTUAL(UFlecsReplicationTransportBase::PublishLayout, );
	virtual void PublishEntity(const FFlecsReplicationRouteKey& Route,
		const FFlecsReplicatedEntitySnapshot& Snapshot) PURE_VIRTUAL(UFlecsReplicationTransportBase::PublishEntity, );
	virtual void RemoveEntity(const FFlecsReplicationRouteKey& Route,
		FFlecsNetworkId NetworkId) PURE_VIRTUAL(UFlecsReplicationTransportBase::RemoveEntity, );
	virtual void HandleProtocolError(const FString& Diagnostic);

	NO_DISCARD UFlecsNetworkWorldSubsystem* GetNetworkSubsystem() const { return NetworkSubsystem.Get(); }

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<UFlecsNetworkWorldSubsystem> NetworkSubsystem;
};

class UNREALFLECS_API FFlecsReplicationTransportRegistry
{
public:
	static bool RegisterProvider(FName ProviderName, UClass* TransportClass);
	static void UnregisterProvider(FName ProviderName);
	static UClass* FindProvider(FName ProviderName);
};
