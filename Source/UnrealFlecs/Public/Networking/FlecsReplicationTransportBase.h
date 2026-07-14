// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Networking/FlecsReplicationTypes.h"

#include "FlecsReplicationTransportBase.generated.h"

class UFlecsNetworkWorldSubsystem;

/**
 * Selects the transport route for an authoritative replicated entity.
 *
 * Routes do not alter the Flecs protocol. They let a transport partition
 * aggregate envelopes into shards, channels, or other delivery groups.
 */
class UNREALFLECS_API IFlecsReplicationRouter
{
public:
	virtual ~IFlecsReplicationRouter() = default;
	virtual FFlecsReplicationRouteKey Route(const FFlecsEntityHandle& Entity) const = 0;
}; // class IFlecsReplicationRouter

/** Default router: every entity uses the single `Default` route. */
class UNREALFLECS_API FFlecsDefaultReplicationRouter final : public IFlecsReplicationRouter
{
public:
	virtual FFlecsReplicationRouteKey Route(const FFlecsEntityHandle&) const override
	{
		return FFlecsReplicationRouteKey::Default();
	}
	
}; // class FFlecsDefaultReplicationRouter final

/**
 * Transport boundary for the Flecs-owned replication protocol.
 *
 * The core calls the publish methods only on authority. A transport receives
 * remote data by enqueueing FFlecsReplicationInboxRecord values on the network
 * subsystem; it must not deserialize component payloads or mutate Flecs
 * entities itself. Publish a layout before a snapshot that references it.
 */
UCLASS(Abstract, Transient)
class UNREALFLECS_API UFlecsReplicationTransportBase : public UObject
{
	GENERATED_BODY()

public:
	/** Stores the owning subsystem. Return false when the active NetDriver is unsupported. */
	virtual bool InitializeTransport(UFlecsNetworkWorldSubsystem* InSubsystem);
	/** Releases transport-owned network objects before the subsystem tears down. */
	virtual void ShutdownTransport();
	/** Optional per-world-tick transport maintenance. */
	virtual void TickTransport() {}
	/** Publishes an immutable structural definition for a route. */
	virtual void PublishLayout(const FFlecsReplicationRouteKey& Route,
		const FFlecsReplicationLayoutDefinition& Layout) PURE_VIRTUAL(UFlecsReplicationTransportBase::PublishLayout, );
	/** Publishes the newest complete state snapshot for one network entity. */
	virtual void PublishEntity(const FFlecsReplicationRouteKey& Route,
		const FFlecsReplicatedEntitySnapshot& Snapshot) PURE_VIRTUAL(UFlecsReplicationTransportBase::PublishEntity, );
	/** Publishes authoritative removal of an entity from a route. */
	virtual void RemoveEntity(const FFlecsReplicationRouteKey& Route,
		FFlecsNetworkId NetworkId) PURE_VIRTUAL(UFlecsReplicationTransportBase::RemoveEntity, );
	/** Handles a rejected remote layout or incompatible serialized protocol input. */
	virtual void HandleProtocolError(const FString& Diagnostic);

	NO_DISCARD UFlecsNetworkWorldSubsystem* GetNetworkSubsystem() const { return NetworkSubsystem.Get(); }

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<UFlecsNetworkWorldSubsystem> NetworkSubsystem;
	
}; // class UFlecsReplicationTransportBase

/** Module-lifetime registry from configured provider names to transport classes. */
class UNREALFLECS_API FFlecsReplicationTransportRegistry
{
public:
	/** Registers a UFlecsReplicationTransportBase subclass under a unique provider name. */
	static bool RegisterProvider(FName ProviderName, UClass* TransportClass);
	/** Removes a provider during its module shutdown. */
	static void UnregisterProvider(FName ProviderName);
	/** Resolves the transport class selected by UFlecsNetworkingModuleSettings. */
	static UClass* FindProvider(FName ProviderName);
}; // class FFlecsReplicationTransportRegistry
