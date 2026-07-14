// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Queue.h"
#include "Networking/FlecsComponentReplicationDescriptor.h"
#include "Networking/FlecsNetworkId.h"

#include "FlecsReplicationTypes.generated.h"

/** Deterministic identity of a complete replicated component/pair composition. */
USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsReplicationLayoutId
{
	GENERATED_BODY()

	FFlecsReplicationLayoutId() = default;
	explicit FFlecsReplicationLayoutId(const FGuid& InValue) 
		: Value(InValue)
	{
	}
	
	NO_DISCARD bool IsValid() const
	{
		return Value.IsValid();
	}
	
	NO_DISCARD FString ToString() const
	{
		return Value.ToString(EGuidFormats::DigitsWithHyphensLower);
	}
	
	friend bool operator==(const FFlecsReplicationLayoutId&, const FFlecsReplicationLayoutId&) = default;
	
	friend uint32 GetTypeHash(const FFlecsReplicationLayoutId& Id)
	{
		return GetTypeHash(Id.Value);
	}

	UPROPERTY()
	FGuid Value;
};

/** Distinguishes a standalone component key from a Flecs pair key. */
UENUM()
enum class EFlecsReplicationKeyKind : uint8
{
	Component,
	Pair
};

/** Portable encoding used for the second element of a replicated pair. */
UENUM()
enum class EFlecsReplicationPairTargetKind : uint8
{
	None,
	Schema,
	StableSymbolValue,
	StablePathValue,
	Entity
};

/**
 * Stable, transport-safe representation of one replicated component or pair.
 *
 * A key describes structure only. When bHasPayload is true, a snapshot carries
 * bytes for it through a FFlecsReplicatedValue indexed by this key's position
 * in the layout. Local FFlecsId values are reconstructed after schema
 * validation on the receiving world.
 */
USTRUCT()
struct UNREALFLECS_API FFlecsReplicationKey
{
	GENERATED_BODY()

	UPROPERTY()
	EFlecsReplicationKeyKind Kind = EFlecsReplicationKeyKind::Component;

	/** Relationship schema; populated only when Kind is Pair. */
	UPROPERTY()
	FFlecsReplicationSchemaId RelationshipSchema;

	UPROPERTY()
	uint32 RelationshipVersion = 0;

	/** Schema whose descriptor supplies the payload storage, if any. */
	UPROPERTY()
	FFlecsReplicationSchemaId StorageSchema;

	UPROPERTY()
	uint32 StorageVersion = 0;

	/** Selects which of the target fields is meaningful for a pair key. */
	UPROPERTY()
	EFlecsReplicationPairTargetKind TargetKind = EFlecsReplicationPairTargetKind::None;

	/** Target component schema when TargetKind is Schema. */
	UPROPERTY()
	FFlecsReplicationSchemaId TargetSchema;

	UPROPERTY()
	uint32 TargetVersion = 0;
	
	/** Peer-common symbol used to resolve a StableValue target on receipt. */
	UPROPERTY()
	FString StableTargetIdentifier;

	/** Target entity identity when TargetKind is Entity. */
	UPROPERTY()
	FFlecsNetworkId EntityTarget;

	/** True when a snapshot contains serialized bytes for this structural key. */
	UPROPERTY()
	bool bHasPayload = false;

	NO_DISCARD FString CanonicalString() const;
	friend bool operator==(const FFlecsReplicationKey&, const FFlecsReplicationKey&) = default;
}; // struct FFlecsReplicationKey

/** Immutable structural definition shared by all snapshots of one Flecs table. */
USTRUCT()
struct UNREALFLECS_API FFlecsReplicationLayoutDefinition
{
	GENERATED_BODY()

	UPROPERTY()
	FFlecsReplicationLayoutId LayoutId;

	UPROPERTY()
	TArray<FFlecsReplicationKey> Keys;
}; // struct FFlecsReplicationLayoutDefinition

/** Transport-facing partition key used by IFlecsReplicationRouter. */
USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsReplicationRouteKey
{
	GENERATED_BODY()
	
	FORCEINLINE friend uint32 GetTypeHash(const FFlecsReplicationRouteKey& Key)
	{
		return GetTypeHash(Key.Name);
	}
	
	static NO_DISCARD FFlecsReplicationRouteKey Default()
	{
		return FFlecsReplicationRouteKey(TEXT("Default"));
	}

	FFlecsReplicationRouteKey() = default;
	explicit FFlecsReplicationRouteKey(const FName InName) : Name(InName) {}
	
	friend bool operator==(const FFlecsReplicationRouteKey&, const FFlecsReplicationRouteKey&) = default;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Networking")
	FName Name = TEXT("Default");
	
}; // struct FFlecsReplicationRouteKey

/** Serialized payload for one payload-bearing layout key. */
USTRUCT()
struct UNREALFLECS_API FFlecsReplicatedValue
{
	GENERATED_BODY()

	UPROPERTY()
	uint16 KeyIndex = 0;

	UPROPERTY()
	TArray<uint8> Bytes;
}; // struct FFlecsReplicatedValue

/**
 * Latest complete replicated state for one entity.
 *
 * CompositionRevision changes only when LayoutId changes. StateRevision is
 * monotonically increased by the authority for every gathered snapshot and is
 * used by clients to discard stale delivery.
 */
USTRUCT()
struct UNREALFLECS_API FFlecsReplicatedEntitySnapshot
{
	GENERATED_BODY()

	UPROPERTY()
	FFlecsNetworkId NetworkId;

	UPROPERTY()
	uint32 StateRevision = 0;

	UPROPERTY()
	uint32 CompositionRevision = 0;

	UPROPERTY()
	FFlecsReplicationLayoutId LayoutId;

	UPROPERTY()
	FFlecsReplicationRouteKey RouteKey;

	UPROPERTY()
	TArray<FFlecsReplicatedValue> Values;
};

/** Kinds of transport-to-core records accepted by the client inbox. */
UENUM()
enum class EFlecsReplicationInboxRecordType : uint8
{
	Layout,
	UpsertEntity,
	RemoveEntity,
	DetachShard
};

/** A queued remote protocol item, tagged with the independent source shard that supplied it. */
struct UNREALFLECS_API FFlecsReplicationInboxRecord
{
	EFlecsReplicationInboxRecordType Type = EFlecsReplicationInboxRecordType::Layout;
	FGuid SourceShard;
	FFlecsReplicationLayoutDefinition Layout;
	FFlecsReplicatedEntitySnapshot Snapshot;
	FFlecsNetworkId NetworkId;
};

/** Thread-safe multi-producer inbox drained by UFlecsNetworkWorldSubsystem on the world tick. */
class UNREALFLECS_API FFlecsReplicationInbox
{
public:
	FORCEINLINE void Enqueue(FFlecsReplicationInboxRecord Record)
	{
		Records.Enqueue(MoveTemp(Record));
	}
	
	FORCEINLINE bool Dequeue(FFlecsReplicationInboxRecord& OutRecord)
	{
		return Records.Dequeue(OutRecord);
	}
	
	NO_DISCARD FORCEINLINE bool IsEmpty() const
	{
		return Records.IsEmpty();
	}

private:
	TQueue<FFlecsReplicationInboxRecord, EQueueMode::Mpsc> Records;
	
}; // class FFlecsReplicationInbox

/**
 * Per-world cache of locally generated and remotely validated layouts.
 *
 * Local layouts are cached by Flecs table because all entities in a table have
 * the same replicated structure. Remote definitions are checked against their
 * deterministic ID before being retained.
 */
class UNREALFLECS_API FFlecsReplicationLayoutRegistry
{
public:
	/** Computes the deterministic layout ID from a sorted key list. */
	static NO_DISCARD FFlecsReplicationLayoutId ComputeLayoutId(const TArray<FFlecsReplicationKey>& Keys);
	
	/** Builds or reuses a local layout for Entity's current Flecs table. */
	const FFlecsReplicationLayoutDefinition* BuildForEntity(const TSolidNotNull<const UFlecsWorld*> World,
		const FFlecsEntityHandle& Entity, bool& bOutWasCreated, FString& OutError);
	
	/** Finds a previously generated or accepted layout definition. */
	NO_DISCARD const FFlecsReplicationLayoutDefinition* Find(FFlecsReplicationLayoutId Id) const;
	/** Adds an already validated remote layout, rejecting identity collisions. */
	bool AddRemoteDefinition(const FFlecsReplicationLayoutDefinition& Definition, FString& OutError);

private:
	TMap<const flecs::table_t*, FFlecsReplicationLayoutId> TableCache;
	TMap<FFlecsReplicationLayoutId, FFlecsReplicationLayoutDefinition> Definitions;
	
}; // class FFlecsReplicationLayoutRegistry
