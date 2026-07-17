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
	
}; // struct FFlecsReplicationLayoutId

/** Distinguishes a standalone component key from a Flecs pair key. */
UENUM()
enum class EFlecsReplicationKeyKind : uint8
{
	Component,
	Pair
}; // enum class EFlecsReplicationKeyKind

/** Portable encoding used for the second element of a replicated pair. */
UENUM()
enum class EFlecsReplicationPairTargetKind : uint8
{
	None,
	Schema,
	StableSymbolValue,
	StablePathValue,
	Entity
}; // enum class EFlecsReplicationPairTargetKind

/**
 * Stable, transport-safe representation of one replicated component or pair.
 *
 * A key describes structure only. When bHasPayload is true, an update carries
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

	/** Schema whose descriptor supplies the payload storage, if any. */
	UPROPERTY()
	FFlecsReplicationSchemaId StorageSchema;

	/** Selects which of the target fields is meaningful for a pair key. */
	UPROPERTY()
	EFlecsReplicationPairTargetKind TargetKind = EFlecsReplicationPairTargetKind::None;

	/** Target component schema when TargetKind is Schema. */
	UPROPERTY()
	FFlecsReplicationSchemaId TargetSchema;
	
	/** Peer-common symbol used to resolve a StableValue target on receipt. */
	UPROPERTY()
	FString StableTargetIdentifier;

	/** Target entity identity when TargetKind is Entity. */
	UPROPERTY()
	FFlecsNetworkId EntityTarget;

	/** True when updates may contain serialized bytes for this structural key. */
	UPROPERTY()
	bool bHasPayload = false;

	/** Identifies the payload codec/quantizer and participates in layout identity. */
	UPROPERTY()
	FString CodecFingerprint = TEXT("None");

	NO_DISCARD FString CanonicalString() const;
	
	friend bool operator==(const FFlecsReplicationKey&, const FFlecsReplicationKey&) = default;
	
}; // struct FFlecsReplicationKey

/** Immutable authoritative structure shared by updates with one composition. */
USTRUCT()
struct UNREALFLECS_API FFlecsReplicationLayoutDefinition
{
	GENERATED_BODY()

	UPROPERTY()
	FFlecsReplicationLayoutId LayoutId;

	UPROPERTY()
	TArray<FFlecsReplicationKey> Keys;
}; // struct FFlecsReplicationLayoutDefinition

/** Built-in audience rules understood by the default interest policy. */
UENUM(BlueprintType)
enum class EFlecsReplicationAudience : uint8
{
	Everyone,
	OwnerOnly,
	Team,
	Zone,
	Custom
}; // enum class EFlecsReplicationAudience

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
		return FFlecsReplicationRouteKey(FName("Default"));
	}

	FFlecsReplicationRouteKey() = default;
	explicit FFlecsReplicationRouteKey(const FName InName) 
		: Name(InName)
	{
	}
	
	friend bool operator==(const FFlecsReplicationRouteKey&, const FFlecsReplicationRouteKey&) = default;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Networking")
	FName Name = FName("Default");
	
}; // struct FFlecsReplicationRouteKey

/**
 * Flecs-owned scheduling and interest metadata for one logical route.
 *
 * Entity and byte limits of zero select the transport defaults. CustomPolicy
 * is evaluated only for the Custom audience.
 */
USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsReplicationRouteDescriptor
{
	GENERATED_BODY()

	static FFlecsReplicationRouteDescriptor Default()
	{
		return {};
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Networking")
	FFlecsReplicationRouteKey LogicalKey = FFlecsReplicationRouteKey::Default();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Networking")
	EFlecsReplicationAudience Audience = EFlecsReplicationAudience::Everyone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Networking")
	FFlecsNetworkId Owner;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Networking")
	int32 Team = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Networking")
	FName Zone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Networking")
	FName CustomPolicy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Networking", meta = (ClampMin = "0.0"))
	float PollFrequency = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Networking", meta = (ClampMin = "0.0"))
	float StaticPriority = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Networking", meta = (ClampMin = "0.0"))
	float SchedulerWeight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Networking", meta = (ClampMin = "0"))
	uint16 PageEntityLimit = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Networking", meta = (ClampMin = "0"))
	uint32 PageByteLimit = 0;

	friend bool operator==(const FFlecsReplicationRouteDescriptor&, const FFlecsReplicationRouteDescriptor&) = default;
}; // struct FFlecsReplicationRouteDescriptor

/** Connection-owned values consumed by the default owner/team/zone interest rules. */
USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsReplicationConnectionInterestContext
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Networking")
	FFlecsNetworkId Owner;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Networking")
	int32 Team = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Networking")
	TSet<FName> Zones;
}; // struct FFlecsReplicationConnectionInterestContext

/** Transport-neutral copy of the connection's current replication viewpoints. */
struct UNREALFLECS_API FFlecsReplicationConnectionView
{
	TArray<FVector> Positions;
	TArray<FVector> Directions;
}; // struct FFlecsReplicationConnectionView

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

/** Whether an entity update replaces all state or only its selected layout keys. */
UENUM()
enum class EFlecsReplicatedEntityUpdateKind : uint8
{
	Full,
	Delta
}; // enum class EFlecsReplicatedEntityUpdateKind

/**
 * Latest complete replicated state for one entity.
 *
 * CompositionRevision changes only when LayoutId changes. StateRevision is
 * monotonically increased by the authority for every committed update and is
 * used by clients to discard stale delivery.
 */
USTRUCT()
struct UNREALFLECS_API FFlecsReplicatedEntityUpdate
{
	GENERATED_BODY()

	static constexpr int32 KeyMaskWordBits = 64;

	void SetKeyChanged(uint16 KeyIndex);
	NO_DISCARD bool IsKeyChanged(uint16 KeyIndex) const;
	NO_DISCARD uint32 GetPayloadByteCount() const;

	UPROPERTY()
	FFlecsNetworkId NetworkId;

	UPROPERTY()
	uint32 StateRevision = 0;

	UPROPERTY()
	uint32 CompositionRevision = 0;

	UPROPERTY()
	EFlecsReplicatedEntityUpdateKind Kind = EFlecsReplicatedEntityUpdateKind::Full;

	UPROPERTY()
	FFlecsReplicationLayoutId LayoutId;

	UPROPERTY()
	FFlecsReplicationRouteDescriptor Route;

	/** One bit per layout key; full updates set every payload-bearing key. */
	UPROPERTY()
	TArray<uint64> ChangedKeyMask;

	UPROPERTY()
	TArray<FFlecsReplicatedValue> Values;

}; // struct FFlecsReplicatedEntityUpdate

/** A bounded fragment of one encoded layout-key value. */
USTRUCT()
struct UNREALFLECS_API FFlecsReplicationUpdateChunk
{
	GENERATED_BODY()

	static constexpr uint16 HeaderKeyIndex = MAX_uint16;
	static constexpr uint32 MaxChunkBytes = 32u * 1024u;
	NO_DISCARD bool IsKeyChanged(uint16 InKeyIndex) const;

	UPROPERTY()
	FFlecsNetworkId NetworkId;

	UPROPERTY()
	uint32 StateRevision = 0;

	UPROPERTY()
	uint32 CompositionRevision = 0;

	UPROPERTY()
	EFlecsReplicatedEntityUpdateKind Kind = EFlecsReplicatedEntityUpdateKind::Full;

	UPROPERTY()
	FFlecsReplicationLayoutId LayoutId;

	UPROPERTY()
	FFlecsReplicationRouteDescriptor Route;

	UPROPERTY()
	TArray<uint64> ChangedKeyMask;

	UPROPERTY()
	uint16 ValueCount = 0;

	UPROPERTY()
	uint16 KeyIndex = HeaderKeyIndex;

	UPROPERTY()
	uint32 Offset = 0;

	UPROPERTY()
	uint32 TotalValueBytes = 0;

	UPROPERTY()
	TArray<uint8> Bytes;
}; // struct FFlecsReplicationUpdateChunk

/** Creates a header plus <=32 KiB value chunks for an update. */
UNREALFLECS_API void BuildFlecsReplicationUpdateChunks(const FFlecsReplicatedEntityUpdate& Update,
	TArray<FFlecsReplicationUpdateChunk>& OutChunks);

/** Kinds of transport-to-core records accepted by the client inbox. */
UENUM()
enum class EFlecsReplicationInboxRecordType : uint8
{
	Layout,
	RemoveLayout,
	UpsertEntity,
	UpdateChunk,
	RemoveEntity,
	DetachShard
}; // enum class EFlecsReplicationInboxRecordType

/** A queued remote protocol item, tagged with the independent source shard that supplied it. */
struct UNREALFLECS_API FFlecsReplicationInboxRecord
{
	EFlecsReplicationInboxRecordType Type = EFlecsReplicationInboxRecordType::Layout;
	FGuid SourceShard;
	FFlecsReplicationLayoutDefinition Layout;
	FFlecsReplicatedEntityUpdate Update;
	FFlecsReplicationUpdateChunk Chunk;
	FFlecsNetworkId NetworkId;
}; // struct FFlecsReplicationInboxRecord

/** Validates and reassembles update chunks independently for every source/entity. */
class UNREALFLECS_API FFlecsReplicationUpdateReassembler
{
public:
	bool Accept(const FGuid& SourceShard, const FFlecsReplicationUpdateChunk& Chunk,
		TOptional<FFlecsReplicatedEntityUpdate>& OutUpdate, FString& OutError);
	void RemoveSource(const FGuid& SourceShard);
	void RemoveEntity(const FGuid& SourceShard, FFlecsNetworkId NetworkId);
	NO_DISCARD bool ReferencesLayout(FFlecsReplicationLayoutId LayoutId) const;
	void Reset();

private:
	struct FKey
	{
		FGuid SourceShard;
		FFlecsNetworkId NetworkId;

		friend bool operator==(const FKey&, const FKey&) = default;
		friend uint32 GetTypeHash(const FKey& Key)
		{
			return HashCombine(GetTypeHash(Key.SourceShard), GetTypeHash(Key.NetworkId));
		}
	}; // struct FKey

	struct FValueAssembly
	{
		uint32 TotalBytes = 0;
		TArray<uint8> Bytes;
		TBitArray<> ReceivedBytes;
		uint32 ReceivedCount = 0;
		bool bReceivedZeroLength = false;
	}; // struct FValueAssembly

	struct FAssembly
	{
		FFlecsReplicatedEntityUpdate Update;
		uint16 ExpectedValueCount = 0;
		bool bReceivedHeader = false;
		bool bReceivedChunkMetadata = false;
		TMap<uint16, FValueAssembly> Values;
	}; // struct FAssembly

	TMap<FKey, FAssembly> Assemblies;
	TMap<FKey, uint32> LatestCompletedRevisions;
}; // class FFlecsReplicationUpdateReassembler

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
 * Definitions are deduplicated by deterministic ID. Layout discovery still
 * probes entity ownership because DontFragment components are not represented
 * by the entity's Flecs table type.
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
	/** Removes a remote definition after the subsystem proves that nothing references it. */
	void RemoveDefinition(FFlecsReplicationLayoutId Id);

private:
	TMap<FFlecsReplicationLayoutId, FFlecsReplicationLayoutDefinition> Definitions;
	
}; // class FFlecsReplicationLayoutRegistry
