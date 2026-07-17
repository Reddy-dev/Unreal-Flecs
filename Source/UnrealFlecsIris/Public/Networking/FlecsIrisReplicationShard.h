// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Iris/ReplicationState/IrisFastArraySerializer.h"
#include "Net/Iris/ReplicationSystem/NetRootObjectAdapter.h"
#include "Net/Iris/ReplicationSystem/NetRootObjectFactory.h"
#include "Networking/FlecsReplicationTypes.h"

#include "FlecsIrisReplicationShard.generated.h"

class UFlecsIrisReplicationShard;

/** Replicated identity and relevance metadata for one stable route page. */
USTRUCT()
struct UNREALFLECSIRIS_API FFlecsIrisReplicationPageDescriptor
{
	GENERATED_BODY()

	UPROPERTY()
	FFlecsReplicationRouteDescriptor Route;

	UPROPERTY()
	uint32 PageIndex = 0;

	UPROPERTY()
	FGuid SourceShardId;
}; // struct FFlecsIrisReplicationPageDescriptor

/** Fast-array item carrying one immutable protocol layout into an Iris page. */
USTRUCT()
struct UNREALFLECSIRIS_API FFlecsIrisLayoutManifestItem : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY()
	FFlecsReplicationLayoutDefinition Definition;

	void PostReplicatedAdd(const struct FFlecsIrisLayoutManifest& Serializer);
	void PostReplicatedChange(const struct FFlecsIrisLayoutManifest& Serializer);
	void PreReplicatedRemove(const struct FFlecsIrisLayoutManifest& Serializer);
}; // struct FFlecsIrisLayoutManifestItem

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
}; // struct FFlecsIrisLayoutManifest

template <>
struct TStructOpsTypeTraits<FFlecsIrisLayoutManifest> : TStructOpsTypeTraitsBase2<FFlecsIrisLayoutManifest>
{
	enum { WithNetDeltaSerializer = true };
};

/** One materialized full baseline, represented as bounded update chunks. */
USTRUCT()
struct UNREALFLECSIRIS_API FFlecsIrisEntityBaselineItem : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FFlecsReplicationUpdateChunk> Chunks;

	void PostReplicatedAdd(const struct FFlecsIrisEntityBaselines& Serializer);
	void PostReplicatedChange(const struct FFlecsIrisEntityBaselines& Serializer);
	void PreReplicatedRemove(const struct FFlecsIrisEntityBaselines& Serializer);
	NO_DISCARD FFlecsNetworkId GetNetworkId() const;
}; // struct FFlecsIrisEntityBaselineItem

/** Current complete state. Existing connections receive the separate update stream. */
USTRUCT()
struct UNREALFLECSIRIS_API FFlecsIrisEntityBaselines : public FIrisFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FFlecsIrisEntityBaselineItem> Items;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FFlecsIrisEntityBaselineItem,
			FFlecsIrisEntityBaselines>(Items, DeltaParams, *this);
	}

	void SetOwner(UFlecsIrisReplicationShard* InOwner) { Owner = InOwner; }
	NO_DISCARD UFlecsIrisReplicationShard* GetOwner() const { return Owner.Get(); }

private:
	TWeakObjectPtr<UFlecsIrisReplicationShard> Owner;
}; // struct FFlecsIrisEntityBaselines

template <>
struct TStructOpsTypeTraits<FFlecsIrisEntityBaselines> : TStructOpsTypeTraitsBase2<FFlecsIrisEntityBaselines>
{
	enum { WithNetDeltaSerializer = true };
};

/** A committed full/delta update for already-relevant connections. */
USTRUCT()
struct UNREALFLECSIRIS_API FFlecsIrisEntityUpdateItem : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FFlecsReplicationUpdateChunk> Chunks;

	void PostReplicatedAdd(const struct FFlecsIrisEntityUpdateStream& Serializer);
	void PostReplicatedChange(const struct FFlecsIrisEntityUpdateStream& Serializer);
}; // struct FFlecsIrisEntityUpdateItem

/** Per-connection Fast Array delta history; entries already contain selected layout keys only. */
USTRUCT()
struct UNREALFLECSIRIS_API FFlecsIrisEntityUpdateStream : public FIrisFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FFlecsIrisEntityUpdateItem> Items;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FFlecsIrisEntityUpdateItem,
			FFlecsIrisEntityUpdateStream>(Items, DeltaParams, *this);
	}

	void SetOwner(UFlecsIrisReplicationShard* InOwner) { Owner = InOwner; }
	NO_DISCARD UFlecsIrisReplicationShard* GetOwner() const { return Owner.Get(); }

private:
	TWeakObjectPtr<UFlecsIrisReplicationShard> Owner;
}; // struct FFlecsIrisEntityUpdateStream

template <>
struct TStructOpsTypeTraits<FFlecsIrisEntityUpdateStream> : TStructOpsTypeTraitsBase2<FFlecsIrisEntityUpdateStream>
{
	enum { WithNetDeltaSerializer = true };
};

/** Aggregate Iris root object for one bounded page of a logical Flecs route. */
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

	void InitializeServer(UWorld* InWorld, const FFlecsReplicationRouteDescriptor& InRoute, uint32 InPageIndex);
	void BindClient(const TSolidNotNull<UWorld*> InWorld);
	bool TryStartReplication();
	void StopReplication();
	void DetachedFromReplication();

	void UpsertLayout(const FFlecsReplicationLayoutDefinition& Layout);
	void UpsertEntity(const FFlecsReplicatedEntityUpdate& Update);
	void RemoveEntity(FFlecsNetworkId NetworkId);
	void SetEntityDormant(FFlecsNetworkId NetworkId, bool bDormant);

	void EnqueueAllReceived();
	void EnqueueReceivedLayout(const FFlecsReplicationLayoutDefinition& Layout) const;
	void EnqueueRemovedLayout(const FFlecsReplicationLayoutDefinition& Layout) const;
	void EnqueueReceivedChunks(const TArray<FFlecsReplicationUpdateChunk>& Chunks) const;
	void EnqueueRemovedEntity(FFlecsNetworkId NetworkId) const;

	NO_DISCARD FGuid GetSourceShardId() const
	{
		return PageDescriptor.SourceShardId;
	}
	NO_DISCARD const FFlecsReplicationRouteDescriptor& GetRouteDescriptor() const { return PageDescriptor.Route; }
	NO_DISCARD uint32 GetPageIndex() const { return PageDescriptor.PageIndex; }
	NO_DISCARD int32 GetLayoutCount() const { return LayoutManifest.Items.Num(); }
	NO_DISCARD int32 GetEntityCount() const { return EntityBaselines.Items.Num(); }
	NO_DISCARD uint32 GetMaterializedPayloadBytes() const { return MaterializedPayloadBytes; }
	NO_DISCARD bool ContainsEntity(FFlecsNetworkId NetworkId) const { return EntityIndices.Contains(NetworkId); }
	NO_DISCARD const FFlecsReplicatedEntityUpdate* FindMaterializedEntity(FFlecsNetworkId NetworkId) const;

private:
	void RemoveLayoutIfUnused(FFlecsReplicationLayoutId LayoutId);
	void RefreshRootDormancy();

	UPROPERTY(Replicated)
	FFlecsIrisReplicationPageDescriptor PageDescriptor;

	UPROPERTY(Replicated)
	FFlecsIrisLayoutManifest LayoutManifest;

	UPROPERTY(Replicated)
	FFlecsIrisEntityBaselines EntityBaselines;

	UPROPERTY(Replicated)
	FFlecsIrisEntityUpdateStream EntityUpdateStream;

	UPROPERTY(Transient)
	TWeakObjectPtr<UWorld> BoundWorld;

	UPROPERTY(Transient)
	TWeakObjectPtr<class UFlecsNetworkWorldSubsystem> NetworkSubsystem;

	TUniquePtr<UE::Net::FNetRootObjectAdapter> RootObjectAdapter;
	TMap<FFlecsReplicationLayoutId, int32> LayoutIndices;
	TMap<FFlecsReplicationLayoutId, int32> LayoutReferenceCounts;
	TMap<FFlecsNetworkId, int32> EntityIndices;
	TMap<FFlecsNetworkId, FFlecsReplicatedEntityUpdate> MaterializedEntities;
	TSet<FFlecsNetworkId> DormantEntities;
	uint32 MaterializedPayloadBytes = 0;
	bool bRootDormant = false;
}; // class UFlecsIrisReplicationShard
