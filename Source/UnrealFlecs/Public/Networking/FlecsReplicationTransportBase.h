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

	virtual FFlecsReplicationRouteDescriptor Route(const FFlecsEntityHandle& Entity) const = 0;
}; // class IFlecsReplicationRouter

/** Default router: every entity uses the single `Default` route. */
class UNREALFLECS_API FFlecsDefaultReplicationRouter final : public IFlecsReplicationRouter
{
public:
	virtual FFlecsReplicationRouteDescriptor Route(const FFlecsEntityHandle& InEntityHandle) const override
	{
		return FFlecsReplicationRouteDescriptor::Default();
	}
	
}; // class FFlecsDefaultReplicationRouter final

/** Immutable inputs supplied to one registered interest policy. */
struct UNREALFLECS_API FFlecsReplicationInterestEvaluationQuery
{
	FFlecsReplicationConnectionId ConnectionId;
	const FFlecsReplicationConnectionInterestContext& Context;
	const FFlecsReplicationConnectionView& View;
}; // struct FFlecsReplicationInterestEvaluationQuery

class UNREALFLECS_API IFlecsReplicationInterestPolicy
{
public:
	virtual ~IFlecsReplicationInterestPolicy() = default;

	virtual FName GetPolicyName() const = 0;
	virtual const UScriptStruct* GetDescriptorStruct() const = 0;
	virtual bool ValidateDescriptor(
		const TInstancedStruct<FFlecsReplicationInterestDescriptorBase>& Descriptor,
		FString& OutError) const = 0;
	virtual bool IsInterested(const TInstancedStruct<FFlecsReplicationInterestDescriptorBase>& Descriptor,
		const FFlecsReplicationInterestEvaluationQuery& Query) const = 0;
}; // class IFlecsReplicationInterestPolicy

template <typename TDescriptor>
class TFlecsReplicationInterestPolicy : public IFlecsReplicationInterestPolicy
{
public:
	static_assert(TIsDerivedFrom<TDescriptor, FFlecsReplicationInterestDescriptorBase>::IsDerived,
		"Interest descriptors must derive from FFlecsReplicationInterestDescriptorBase");

	explicit TFlecsReplicationInterestPolicy(const FName InPolicyName)
		: PolicyName(InPolicyName)
	{
	}

	virtual FName GetPolicyName() const final
	{
		return PolicyName;
	}

	virtual const UScriptStruct* GetDescriptorStruct() const final
	{
		return TDescriptor::StaticStruct();
	}

	virtual bool ValidateDescriptor(
		const TInstancedStruct<FFlecsReplicationInterestDescriptorBase>& Descriptor,
		FString& OutError) const final
	{
		if (Descriptor.GetScriptStruct() != TDescriptor::StaticStruct())
		{
			OutError = FString::Printf(TEXT("Policy '%s' expects descriptor '%s', received '%s'"),
				*PolicyName.ToString(), *TDescriptor::StaticStruct()->GetPathName(),
				Descriptor.GetScriptStruct() ? *Descriptor.GetScriptStruct()->GetPathName() : TEXT("None"));
			return false;
		}

		const TDescriptor* Typed = Descriptor.template GetPtr<TDescriptor>();
		return Typed && ValidateTypedDescriptor(*Typed, OutError);
	}

	virtual bool IsInterested(
		const TInstancedStruct<FFlecsReplicationInterestDescriptorBase>& Descriptor,
		const FFlecsReplicationInterestEvaluationQuery& Query) const final
	{
		const TDescriptor* Typed = Descriptor.template GetPtr<TDescriptor>();
		return Descriptor.GetScriptStruct() == TDescriptor::StaticStruct() && Typed
			&& IsInterested(*Typed, Query);
	}

protected:
	virtual bool ValidateTypedDescriptor(const TDescriptor&, FString&) const
	{
		return true;
	}

	virtual bool IsInterested(const TDescriptor& Descriptor,
		const FFlecsReplicationInterestEvaluationQuery& Query) const = 0;

private:
	FName PolicyName;
}; // class TFlecsReplicationInterestPolicy

class UNREALFLECS_API FFlecsEveryoneReplicationInterestPolicy final : public TFlecsReplicationInterestPolicy<FFlecsReplicationEveryoneInterestDescriptor>
{
public:
	FFlecsEveryoneReplicationInterestPolicy();

protected:
	virtual bool IsInterested(const FFlecsReplicationEveryoneInterestDescriptor& Descriptor,
		const FFlecsReplicationInterestEvaluationQuery& Query) const override;
}; // class FFlecsEveryoneReplicationInterestPolicy

class UNREALFLECS_API FFlecsOwnerReplicationInterestPolicy final : public TFlecsReplicationInterestPolicy<FFlecsReplicationOwnerInterestDescriptor>
{
public:
	FFlecsOwnerReplicationInterestPolicy();

protected:
	virtual bool ValidateTypedDescriptor(const FFlecsReplicationOwnerInterestDescriptor& Descriptor, FString& OutError) const override;
	
	virtual bool IsInterested(const FFlecsReplicationOwnerInterestDescriptor& Descriptor,
		const FFlecsReplicationInterestEvaluationQuery& Query) const override;
	
}; // class FFlecsOwnerReplicationInterestPolicy

class UNREALFLECS_API FFlecsSpatialCellReplicationInterestPolicy final
	: public TFlecsReplicationInterestPolicy<FFlecsReplicationSpatialCellInterestDescriptor>
{
public:
	FFlecsSpatialCellReplicationInterestPolicy();

protected:
	virtual bool ValidateTypedDescriptor(const FFlecsReplicationSpatialCellInterestDescriptor& Descriptor,
		FString& OutError) const override;
	virtual bool IsInterested(const FFlecsReplicationSpatialCellInterestDescriptor& Descriptor,
		const FFlecsReplicationInterestEvaluationQuery& Query) const override;
	
}; // class FFlecsSpatialCellReplicationInterestPolicy

class UNREALFLECS_API FFlecsReplicationInterestPolicyRegistry
{
public:
	static bool RegisterPolicy(TUniquePtr<IFlecsReplicationInterestPolicy> Policy);
	static bool UnregisterPolicy(FName PolicyName);
	static NO_DISCARD const IFlecsReplicationInterestPolicy* FindPolicy(FName PolicyName);
	static bool ValidateBinding(const FFlecsReplicationInterestBinding& Binding, OUT FString& OutError);
	
}; // class FFlecsReplicationInterestPolicyRegistry

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
	virtual void PublishLayout(const FFlecsReplicationRouteDescriptor& Route,
		const FFlecsReplicationLayoutDefinition& Layout) PURE_VIRTUAL(UFlecsReplicationTransportBase::PublishLayout, );
	
	/** Publishes the newest complete state snapshot for one network entity. */
	virtual void PublishEntity(const FFlecsReplicationRouteDescriptor& Route,
		const FFlecsReplicatedEntityUpdate& Update) PURE_VIRTUAL(UFlecsReplicationTransportBase::PublishEntity, );

	/** Publishes the destination full baseline before removing the old source. */
	virtual void MigrateEntity(const FFlecsReplicationRouteDescriptor& OldRoute,
		const FFlecsReplicationRouteDescriptor& NewRoute, const FFlecsReplicationLayoutDefinition& Layout,
		const FFlecsReplicatedEntityUpdate& FullUpdate);
	
	/** Publishes authoritative removal of an entity from a route. */
	virtual void RemoveEntity(const FFlecsReplicationRouteDescriptor& Route,
		FFlecsNetworkId NetworkId) PURE_VIRTUAL(UFlecsReplicationTransportBase::RemoveEntity, );
	
	/** Handles a rejected remote layout or incompatible serialized protocol input. */
	virtual void HandleProtocolError(const FString& Diagnostic);

	NO_DISCARD FORCEINLINE UFlecsNetworkWorldSubsystem* GetNetworkSubsystem() const
	{
		return NetworkSubsystem.Get();
	}

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
	static NO_DISCARD UClass* FindProvider(FName ProviderName);
	
}; // class FFlecsReplicationTransportRegistry
