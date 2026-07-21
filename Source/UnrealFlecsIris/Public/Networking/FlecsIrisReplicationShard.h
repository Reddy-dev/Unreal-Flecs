// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Iris/ReplicationState/IrisFastArraySerializer.h"
#include "Net/Iris/ReplicationSystem/NetRootObjectAdapter.h"
#include "Net/Iris/ReplicationSystem/NetRootObjectFactory.h"
#include "Networking/FlecsReplicationTypes.h"

#include "FlecsIrisReplicationShard.generated.h"

class UFlecsIrisReplicationShard;

/** Fast-array item carrying one immutable protocol layout into an Iris shard. */
USTRUCT()
struct UNREALFLECSIRIS_API FFlecsIrisLayoutManifestItem : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY()
	FFlecsReplicationLayoutDefinition Definition;

	void PostReplicatedAdd(const struct FFlecsIrisLayoutManifest& Serializer);
	void PostReplicatedChange(const struct FFlecsIrisLayoutManifest& Serializer);
};

/** Push-based fast array of layouts required before a snapshot can be applied. */
USTRUCT()
struct UNREALFLECSIRIS_API FFlecsIrisLayoutManifest : public FIrisFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FFlecsIrisLayoutManifestItem> Items;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FFlecsIrisLayoutManifestItem,
			FFlecsIrisLayoutManifest>(Items, DeltaParams, *this);
	}

	void SetOwner(UFlecsIrisReplicationShard* InOwner)
	{
		Owner = InOwner;
	}
	
	NO_DISCARD UFlecsIrisReplicationShard* GetOwner() const
	{
		return Owner.Get();
	}

private:
	TWeakObjectPtr<UFlecsIrisReplicationShard> Owner;
};

template <> 
struct TStructOpsTypeTraits<FFlecsIrisLayoutManifest> : TStructOpsTypeTraitsBase2<FFlecsIrisLayoutManifest>
{
	enum
	{
		WithNetDeltaSerializer = true
	};
};

/** Small materialized entity header. Payload bytes live in FFlecsIrisEntityValues. */
USTRUCT()
struct UNREALFLECSIRIS_API FFlecsIrisEntityHeaderItem : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY()
	FFlecsReplicatedEntityUpdate Header;

	void PostReplicatedAdd(const struct FFlecsIrisEntityHeaders& Serializer);
	void PostReplicatedChange(const struct FFlecsIrisEntityHeaders& Serializer);
	void PreReplicatedRemove(const struct FFlecsIrisEntityHeaders& Serializer);
};

/** Push-based latest header per entity. */
USTRUCT()
struct UNREALFLECSIRIS_API FFlecsIrisEntityHeaders : public FIrisFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FFlecsIrisEntityHeaderItem> Items;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FFlecsIrisEntityHeaderItem,
			FFlecsIrisEntityHeaders>(Items, DeltaParams, *this);
	}

	void SetOwner(UFlecsIrisReplicationShard* InOwner)
	{
		Owner = InOwner;
	}

	NO_DISCARD UFlecsIrisReplicationShard* GetOwner() const
	{
		return Owner.Get();
	}

private:
	TWeakObjectPtr<UFlecsIrisReplicationShard> Owner;
};

template <>
struct TStructOpsTypeTraits<FFlecsIrisEntityHeaders> : TStructOpsTypeTraitsBase2<FFlecsIrisEntityHeaders>
{
	enum
	{
		WithNetDeltaSerializer = true
	};
};

USTRUCT()
struct UNREALFLECSIRIS_API FFlecsIrisEntityValueItem : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY()
	FFlecsNetworkId NetworkId;

	UPROPERTY()
	FFlecsReplicationLayoutId LayoutId;

	UPROPERTY()
	uint32 StateRevision = 0;

	UPROPERTY()
	FFlecsReplicatedValue Value;

	void PostReplicatedAdd(const struct FFlecsIrisEntityValues& Serializer);
	void PostReplicatedChange(const struct FFlecsIrisEntityValues& Serializer);
}; // struct FFlecsIrisEntityValueItem

USTRUCT()
struct UNREALFLECSIRIS_API FFlecsIrisEntityValues : public FIrisFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FFlecsIrisEntityValueItem> Items;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FFlecsIrisEntityValueItem,
			FFlecsIrisEntityValues>(Items, DeltaParams, *this);
	}

	void SetOwner(UFlecsIrisReplicationShard* InOwner)
	{
		Owner = InOwner;
	}

	NO_DISCARD UFlecsIrisReplicationShard* GetOwner() const
	{
		return Owner.Get();
	}

private:
	TWeakObjectPtr<UFlecsIrisReplicationShard> Owner;
}; // struct FFlecsIrisEntityValues

template <>
struct TStructOpsTypeTraits<FFlecsIrisEntityValues> : TStructOpsTypeTraitsBase2<FFlecsIrisEntityValues>
{
	enum
	{
		WithNetDeltaSerializer = true
	};
};

/**
 * Aggregate Iris root object for a single Flecs replication route.
 *
 * The authority keeps a layout manifest plus materialized headers and values.
 * entity. On clients, the root-object factory binds the shard to the world
 * subsystem and forwards fast-array events into its protocol inbox.
 */
UCLASS(Transient)
class UNREALFLECSIRIS_API UFlecsIrisReplicationShard : public UObject, public INetRootObjectFactoryExtension
{
	GENERATED_BODY()

public:
	virtual UWorld* GetWorld() const override;
	
	virtual bool IsSupportedForNetworking() const override
	{
		return true;
	}
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context,
		UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
	virtual void FillRootObjectReplicationParams(const UE::Net::FRootObjectReplicationParamsContext& Context,
		UE::Net::FRootObjectReplicationParams& OutParams) const override;

	/** Configures this instance as one bounded authority-side page. */
	void InitializeServer(UWorld* InWorld, const FFlecsReplicationRouteDescriptor& InRoute,
		float InPollFrequency, float InStaticPriority, uint16 InEntityLimit, uint32 InByteLimit);
	
	/** Binds a factory-created client instance to its destination world subsystem. */
	void BindClient(const TSolidNotNull<UWorld*> InWorld);
	
	/** Attempts to attach the authority-side root object to the active Iris replication system. */
	bool TryStartReplication();
	
	/** Stops and destroys the Iris root-object adapter while preserving no transport state. */
	void StopReplication();
	
	/** Enqueues a detach record so the client removes entities sourced by this shard. */
	void DetachedFromReplication();

	void UpsertLayout(const FFlecsReplicationLayoutDefinition& Layout);
	void UpsertEntity(const FFlecsReplicatedEntityUpdate& Update);
	void RemoveEntity(FFlecsNetworkId NetworkId);
	NO_DISCARD bool CanFitUpdate(const FFlecsReplicatedEntityUpdate& Update) const;
	NO_DISCARD bool IsEmpty() const
	{
		return EntityHeaders.Items.IsEmpty();
	}

	NO_DISCARD const FFlecsReplicationRouteDescriptor& GetRouteDescriptor() const
	{
		return RouteDescriptor;
	}
	
	/** Re-enqueues full current state after a late client binds to the shard. */
	void EnqueueAllReceived();
	void EnqueueReceivedLayout(const FFlecsReplicationLayoutDefinition& Layout) const;
	void TryEnqueueReceivedEntity(FFlecsNetworkId NetworkId);
	void EnqueueRemovedEntity(FFlecsNetworkId NetworkId) const;

	NO_DISCARD FGuid GetSourceShardId() const;
	
	NO_DISCARD FORCEINLINE int32 GetLayoutCount() const
	{
		return LayoutManifest.Items.Num();
	}
	
	NO_DISCARD FORCEINLINE int32 GetEntityCount() const
	{
		return EntityHeaders.Items.Num();
	}

#if WITH_AUTOMATION_TESTS || WITH_EDITOR
	NO_DISCARD const FFlecsReplicatedEntityUpdate* FindHeaderForTesting(FFlecsNetworkId NetworkId) const
	{
		const int32* Index = EntityIndices.Find(NetworkId);
		return Index ? &EntityHeaders.Items[*Index].Header : nullptr;
	}
#endif

private:
	bool TryAttachInterestFilter();

	UPROPERTY(Replicated)
	FFlecsReplicationRouteDescriptor RouteDescriptor;

	UPROPERTY(Replicated)
	FGuid SourceShardId;

	UPROPERTY(Replicated)
	FFlecsIrisLayoutManifest LayoutManifest;

	UPROPERTY(Replicated)
	FFlecsIrisEntityHeaders EntityHeaders;

	UPROPERTY(Replicated)
	FFlecsIrisEntityValues EntityValues;

	UPROPERTY(Transient)
	TWeakObjectPtr<UWorld> BoundWorld;

	UPROPERTY(Transient)
	TWeakObjectPtr<class UFlecsNetworkWorldSubsystem> NetworkSubsystem;

	TUniquePtr<UE::Net::FNetRootObjectAdapter> RootObjectAdapter;
	
	UPROPERTY()
	TMap<FFlecsReplicationLayoutId, int32> LayoutIndices;
	
	UPROPERTY()
	TMap<FFlecsNetworkId, int32> EntityIndices;
	TMap<FFlecsNetworkId, TMap<uint16, int32>> EntityValueIndices;
	TMap<FFlecsNetworkId, uint32> EntityPayloadBytes;
	TSet<FFlecsNetworkId> ReceivedBaselines;
	uint32 RetainedPayloadBytes = 0;
	uint16 EntityLimit = 256;
	uint32 ByteLimit = 256 * 1024;
	bool bWarnedOversizeEntity = false;
	bool bWarnedInvalidInterest = false;
	bool bInterestFilterAttached = false;
	
	float PollFrequency = 20.0f;
	float StaticPriority = 1.0f;
	
}; // class UFlecsIrisReplicationShard
