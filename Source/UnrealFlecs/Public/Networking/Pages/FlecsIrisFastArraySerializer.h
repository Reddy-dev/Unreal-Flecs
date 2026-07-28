// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Iris/ReplicationState/IrisFastArraySerializer.h"

#include "Networking/FlecsNetworkId.h"
#include "Networking/Layout/FlecsReplicationSnapshot.h"

#include "FlecsIrisFastArraySerializer.generated.h"

class UFlecsNetEntityPage;

USTRUCT()
struct UNREALFLECS_API FFlecsNetEntityPageItem : public FFastArraySerializerItem
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FFlecsNetworkId NetworkId;
	
	UPROPERTY()
	FFlecsEntityReplicationSnapshot Snapshot;
	
}; // struct FFlecsNetEntityPageItem

USTRUCT()
struct UNREALFLECS_API FFlecsNetEntityPageArray : public FIrisFastArraySerializer
{
	GENERATED_BODY()
	
public:
	FFlecsNetEntityPageArray()
		: Owner(nullptr)
	{
	}

	void SetOwner(const TSolidNotNull<UFlecsNetEntityPage*> InOwner);

	UPROPERTY()
	TArray<FFlecsNetEntityPageItem> Items;
	
	UPROPERTY()
	TWeakObjectPtr<UFlecsNetEntityPage> Owner;
	
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms);
	
	void PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize);
	
}; // struct FFlecsNetEntityPageArray

template<>
struct TStructOpsTypeTraits<FFlecsNetEntityPageArray> : public TStructOpsTypeTraitsBase2<FFlecsNetEntityPageArray>
{
	enum
	{
		WithNetDeltaSerializer = true
	};
}; // struct TStructOpsTypeTraits<FFlecsNetEntityPageArray>