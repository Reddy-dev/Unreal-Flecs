// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Iris/ReplicationState/IrisFastArraySerializer.h"
#include "Net/Iris/ReplicationSystem/NetRootObjectAdapter.h"
#include "Net/Iris/ReplicationSystem/NetRootObjectFactory.h"
#include "Networking/FlecsReplicationTypes.h"

#include "FlecsIrisReplicationShard.generated.h"

class UFlecsIrisReplicationShard;

USTRUCT()
struct UNREALFLECSIRIS_API FFlecsIrisLayoutManifestItem : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY()
	FFlecsReplicationLayoutDefinition Definition;

	void PostReplicatedAdd(const struct FFlecsIrisLayoutManifest& Serializer);
	void PostReplicatedChange(const struct FFlecsIrisLayoutManifest& Serializer);
};

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

	void InitializeServer(UWorld* InWorld, const FFlecsReplicationRouteKey& InShardKey,
		float InPollFrequency, float InStaticPriority);
	void BindClient(UWorld* InWorld);
	bool TryStartReplication();
	void StopReplication();
	void DetachedFromReplication();

	void UpsertLayout(const FFlecsReplicationLayoutDefinition& Layout);
	void UpsertEntity(const FFlecsReplicatedEntitySnapshot& Snapshot);
	void RemoveEntity(FFlecsNetworkId NetworkId);
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
