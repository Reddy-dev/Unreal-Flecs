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

	void SetOwner(UFlecsIrisReplicationShard* InOwner) { Owner = InOwner; }
	NO_DISCARD UFlecsIrisReplicationShard* GetOwner() const { return Owner.Get(); }

private:
	TWeakObjectPtr<UFlecsIrisReplicationShard> Owner;
};

template<> struct TStructOpsTypeTraits<FFlecsIrisLayoutManifest>
	: TStructOpsTypeTraitsBase2<FFlecsIrisLayoutManifest>
{
	enum { WithNetDeltaSerializer = true };
};

/** Fast-array item holding the latest complete snapshot for one Flecs network ID. */
USTRUCT()
struct UNREALFLECSIRIS_API FFlecsIrisEntitySnapshotItem : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY()
	FFlecsReplicatedEntitySnapshot Snapshot;

	void PostReplicatedAdd(const struct FFlecsIrisEntitySnapshots& Serializer);
	void PostReplicatedChange(const struct FFlecsIrisEntitySnapshots& Serializer);
	void PreReplicatedRemove(const struct FFlecsIrisEntitySnapshots& Serializer);
};

/** Push-based fast array of latest entity snapshots within an aggregate shard. */
USTRUCT()
struct UNREALFLECSIRIS_API FFlecsIrisEntitySnapshots : public FIrisFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FFlecsIrisEntitySnapshotItem> Items;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FFlecsIrisEntitySnapshotItem,
			FFlecsIrisEntitySnapshots>(Items, DeltaParams, *this);
	}

	void SetOwner(UFlecsIrisReplicationShard* InOwner) { Owner = InOwner; }
	NO_DISCARD UFlecsIrisReplicationShard* GetOwner() const { return Owner.Get(); }

private:
	TWeakObjectPtr<UFlecsIrisReplicationShard> Owner;
};

template<> struct TStructOpsTypeTraits<FFlecsIrisEntitySnapshots>
	: TStructOpsTypeTraitsBase2<FFlecsIrisEntitySnapshots>
{
	enum { WithNetDeltaSerializer = true };
};

/**
 * Aggregate Iris root object for a single Flecs replication route.
 *
 * The authority keeps a layout manifest and the latest snapshot for each
 * entity. On clients, the root-object factory binds the shard to the world
 * subsystem and forwards fast-array events into its protocol inbox.
 */
UCLASS(Transient)
class UNREALFLECSIRIS_API UFlecsIrisReplicationShard : public UObject, public INetRootObjectFactoryExtension
{
	GENERATED_BODY()

public:
	virtual UWorld* GetWorld() const override;
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context,
		UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
	virtual void FillRootObjectReplicationParams(const UE::Net::FRootObjectReplicationParamsContext& Context,
		UE::Net::FRootObjectReplicationParams& OutParams) const override;

	/** Configures this instance as the authority-side root object for InShardKey. */
	void InitializeServer(UWorld* InWorld, const FFlecsReplicationRouteKey& InShardKey,
		float InPollFrequency, float InStaticPriority);
	/** Binds a factory-created client instance to its destination world subsystem. */
	void BindClient(UWorld* InWorld);
	/** Attempts to attach the authority-side root object to the active Iris replication system. */
	bool TryStartReplication();
	/** Stops and destroys the Iris root-object adapter while preserving no transport state. */
	void StopReplication();
	/** Enqueues a detach record so the client removes entities sourced by this shard. */
	void DetachedFromReplication();

	void UpsertLayout(const FFlecsReplicationLayoutDefinition& Layout);
	void UpsertEntity(const FFlecsReplicatedEntitySnapshot& Snapshot);
	void RemoveEntity(FFlecsNetworkId NetworkId);
	/** Re-enqueues full current state after a late client binds to the shard. */
	void EnqueueAllReceived();
	void EnqueueReceivedLayout(const FFlecsReplicationLayoutDefinition& Layout) const;
	void EnqueueReceivedEntity(const FFlecsReplicatedEntitySnapshot& Snapshot) const;
	void EnqueueRemovedEntity(FFlecsNetworkId NetworkId) const;

	NO_DISCARD FGuid GetSourceShardId() const;
	NO_DISCARD int32 GetLayoutCount() const { return LayoutManifest.Items.Num(); }
	NO_DISCARD int32 GetEntityCount() const { return EntitySnapshots.Items.Num(); }

private:
	UPROPERTY(Replicated)
	FFlecsReplicationRouteKey ShardKey;

	UPROPERTY(Replicated)
	FFlecsIrisLayoutManifest LayoutManifest;

	UPROPERTY(Replicated)
	FFlecsIrisEntitySnapshots EntitySnapshots;

	UPROPERTY(Transient)
	TWeakObjectPtr<UWorld> BoundWorld;

	UPROPERTY(Transient)
	TWeakObjectPtr<class UFlecsNetworkWorldSubsystem> NetworkSubsystem;

	TUniquePtr<UE::Net::FNetRootObjectAdapter> RootObjectAdapter;
	TMap<FFlecsReplicationLayoutId, int32> LayoutIndices;
	TMap<FFlecsNetworkId, int32> EntityIndices;
	float PollFrequency = 20.0f;
	float StaticPriority = 1.0f;
}; // class UFlecsIrisReplicationShard
