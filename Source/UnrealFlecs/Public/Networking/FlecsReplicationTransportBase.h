// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Networking/FlecsReplicationRouting.h"
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
		if (const FFlecsReplicationRouting* Routing = InEntityHandle.TryGet<FFlecsReplicationRouting>())
		{
			return Routing->Route;
		}
		return FFlecsReplicationRouteDescriptor::Default();
	}
	
}; // class FFlecsDefaultReplicationRouter final

/** Immutable inputs supplied to one interest-policy evaluation. */
struct UNREALFLECS_API FFlecsReplicationInterestEvaluationQuery
{
	uint32 ConnectionId = 0;
	const FFlecsReplicationConnectionInterestContext& Context;
	const FFlecsReplicationConnectionView& View;
}; // struct FFlecsReplicationInterestEvaluationQuery

/** Type-erased registration surface for one named, descriptor-owned policy. */
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

/** Checked type erasure for game and prebuilt policies with concrete descriptors. */
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

		const TDescriptor* TypedDescriptor = Descriptor.template GetPtr<TDescriptor>();
		return TypedDescriptor && ValidateDescriptor(*TypedDescriptor, OutError);
	}

	virtual bool IsInterested(
		const TInstancedStruct<FFlecsReplicationInterestDescriptorBase>& Descriptor,
		const FFlecsReplicationInterestEvaluationQuery& Query) const final
	{
		const TDescriptor* TypedDescriptor = Descriptor.template GetPtr<TDescriptor>();
		return Descriptor.GetScriptStruct() == TDescriptor::StaticStruct() && TypedDescriptor
			&& IsInterested(*TypedDescriptor, Query);
	}

protected:
	virtual bool ValidateDescriptor(const TDescriptor&, FString&) const
	{
		return true;
	}

	virtual bool IsInterested(const TDescriptor& Descriptor,
		const FFlecsReplicationInterestEvaluationQuery& Query) const = 0;

private:
	FName PolicyName;
}; // class TFlecsReplicationInterestPolicy

class UNREALFLECS_API FFlecsEveryoneReplicationInterestPolicy final
	: public TFlecsReplicationInterestPolicy<FFlecsReplicationEveryoneInterestDescriptor>
{
public:
	FFlecsEveryoneReplicationInterestPolicy();

protected:
	virtual bool IsInterested(const FFlecsReplicationEveryoneInterestDescriptor& Descriptor,
		const FFlecsReplicationInterestEvaluationQuery& Query) const override;
}; // class FFlecsEveryoneReplicationInterestPolicy

class UNREALFLECS_API FFlecsOwnerReplicationInterestPolicy final
	: public TFlecsReplicationInterestPolicy<FFlecsReplicationOwnerInterestDescriptor>
{
public:
	FFlecsOwnerReplicationInterestPolicy();

protected:
	virtual bool ValidateDescriptor(const FFlecsReplicationOwnerInterestDescriptor& Descriptor,
		FString& OutError) const override;
	virtual bool IsInterested(const FFlecsReplicationOwnerInterestDescriptor& Descriptor,
		const FFlecsReplicationInterestEvaluationQuery& Query) const override;
}; // class FFlecsOwnerReplicationInterestPolicy

class UNREALFLECS_API FFlecsZoneReplicationInterestPolicy final
	: public TFlecsReplicationInterestPolicy<FFlecsReplicationZoneInterestDescriptor>
{
public:
	FFlecsZoneReplicationInterestPolicy();

protected:
	virtual bool ValidateDescriptor(const FFlecsReplicationZoneInterestDescriptor& Descriptor,
		FString& OutError) const override;
	virtual bool IsInterested(const FFlecsReplicationZoneInterestDescriptor& Descriptor,
		const FFlecsReplicationInterestEvaluationQuery& Query) const override;
}; // class FFlecsZoneReplicationInterestPolicy

/** Module-lifetime policy registry shared by worlds and transport adapters. */
class UNREALFLECS_API FFlecsReplicationInterestPolicyRegistry
{
public:
	static bool RegisterPolicy(TUniquePtr<IFlecsReplicationInterestPolicy> Policy);
	static bool UnregisterPolicy(FName PolicyName);
	static NO_DISCARD const IFlecsReplicationInterestPolicy* FindPolicy(FName PolicyName);
	static bool ValidateBinding(const FFlecsReplicationInterestBinding& Binding, FString& OutError);
}; // class FFlecsReplicationInterestPolicyRegistry

UENUM(BlueprintType)
enum class EFlecsReplicationDormancyMode : uint8
{
	Automatic,
	ForceAwake,
	DormantUntilDirty
}; // enum class EFlecsReplicationDormancyMode

/**
 * Transport boundary for the Flecs-owned replication protocol.
 *
 * The core calls the publish methods only on authority. A transport receives
 * remote data by enqueueing FFlecsReplicationInboxRecord values on the network
 * subsystem; it must not deserialize component payloads or mutate Flecs
 * entities itself. Publish a layout before an update that references it.
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
	
	/** Publishes one full baseline or same-layout component delta. */
	virtual void PublishEntity(const FFlecsReplicationRouteDescriptor& Route,
		const FFlecsReplicatedEntityUpdate& Update) PURE_VIRTUAL(UFlecsReplicationTransportBase::PublishEntity, );

	/** Commits a full baseline to the new route before removing the old source. */
	virtual void MigrateEntity(const FFlecsReplicationRouteDescriptor& OldRoute,
		const FFlecsReplicationRouteDescriptor& NewRoute, const FFlecsReplicationLayoutDefinition& Layout,
		const FFlecsReplicatedEntityUpdate& FullUpdate);
	
	/** Publishes authoritative removal of an entity from a route. */
	virtual void RemoveEntity(const FFlecsReplicationRouteDescriptor& Route,
		FFlecsNetworkId NetworkId) PURE_VIRTUAL(UFlecsReplicationTransportBase::RemoveEntity, );

	/** Updates page-level dormancy bookkeeping for an entity. */
	virtual void SetEntityDormancy(const FFlecsReplicationRouteDescriptor& Route,
		FFlecsNetworkId NetworkId, bool bDormant) {}
	
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
