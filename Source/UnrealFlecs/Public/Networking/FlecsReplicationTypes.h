// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Queue.h"
#include "Networking/FlecsComponentReplicationDescriptor.h"
#include "Networking/FlecsNetworkId.h"
#include "StructUtils/InstancedStruct.h"

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

/** Portable encoding used for either individual element of a replicated ID. */
UENUM()
enum class EFlecsReplicationPairTargetKind : uint8
{
	// @TODO: Remove None
	None,
	Schema,
	StableSymbolValue,
	StablePathValue,
	Entity
}; // enum class EFlecsReplicationPairTargetKind

USTRUCT()
struct UNREALFLECS_API FFlecsReplicationIndividualKey
{
	GENERATED_BODY()

	UPROPERTY()
	EFlecsReplicationPairTargetKind Kind = EFlecsReplicationPairTargetKind::None;
	
	UPROPERTY()
	FFlecsReplicationSchemaId Schema;
	
	UPROPERTY()
	FString StableIdentifier;
	
	UPROPERTY()
	FFlecsNetworkId Entity;
	
	FORCEINLINE friend bool operator==(const FFlecsReplicationIndividualKey&, const FFlecsReplicationIndividualKey&) = default;
	
	FORCEINLINE friend uint32 GetTypeHash(const FFlecsReplicationIndividualKey& Key)
	{
		uint32 Hash = GetTypeHash(Key.Kind);
		
		switch (Key.Kind)
		{
			case EFlecsReplicationPairTargetKind::None:
				break;
			case EFlecsReplicationPairTargetKind::Schema:
				Hash = HashCombine(Hash, GetTypeHash(Key.Schema));
				break;
			case EFlecsReplicationPairTargetKind::StableSymbolValue:
			case EFlecsReplicationPairTargetKind::StablePathValue:
				Hash = HashCombine(Hash, GetTypeHash(Key.StableIdentifier));
				break;
			case EFlecsReplicationPairTargetKind::Entity:
				Hash = HashCombine(Hash, GetTypeHash(Key.Entity));
				break;
		}
		
		return Hash;
	}
	
	NO_DISCARD FString CanonicalString() const;
	
	NO_DISCARD const FFlecsComponentReplicationDescriptor* TryGetDescriptor(const TSolidNotNull<const UFlecsWorld*> InWorld) const;
	
}; // struct FFlecsReplicationIndividualKey

UENUM()
enum class EFlecsReplicationKeyStorageKind : uint8
{
	None,
	Primary,
	Secondary
}; // enum class EFlecsReplicationKeyStorageKind

/**
 * Stable, transport-safe representation of one replicated component or pair.
 *
 * A key always describes structure. When StorageKind selects an individual, a
 * snapshot may carry bytes through a FFlecsReplicatedValue indexed by this
 * key's position in the layout. Local FFlecsId values are reconstructed after
 * identity validation on the receiving world.
 */
USTRUCT()
struct UNREALFLECS_API FFlecsReplicationKey
{
	GENERATED_BODY()

	UPROPERTY()
	EFlecsReplicationKeyKind Kind = EFlecsReplicationKeyKind::Component;
	
	UPROPERTY()
	EFlecsReplicationKeyStorageKind StorageKind = EFlecsReplicationKeyStorageKind::None;

	/** Standalone ID, or the first element when Kind is Pair. */
	UPROPERTY()
	FFlecsReplicationIndividualKey Primary;

	/** Second element when Kind is Pair. */
	UPROPERTY()
	FFlecsReplicationIndividualKey Secondary;

	NO_DISCARD FString CanonicalString() const;
	
	friend bool operator==(const FFlecsReplicationKey&, const FFlecsReplicationKey&) = default;
	
	NO_DISCARD const FFlecsComponentReplicationDescriptor* TryGetStorageDescriptor(const TSolidNotNull<const UFlecsWorld*> InWorld) const;
	
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

/** Opaque transport connection identity. Iris adapters use the parent connection ID. */
USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsReplicationConnectionId
{
	GENERATED_BODY()

	FFlecsReplicationConnectionId() = default;
	explicit FFlecsReplicationConnectionId(const uint32 InValue)
		: Value(InValue)
	{
	}

	NO_DISCARD bool IsValid() const
	{
		return Value != 0;
	}

	friend bool operator==(const FFlecsReplicationConnectionId&, const FFlecsReplicationConnectionId&) = default;

	UPROPERTY(EditAnywhere, Category = "Flecs | Networking")
	uint32 Value = 0;
}; // struct FFlecsReplicationConnectionId

USTRUCT()
struct UNREALFLECS_API FFlecsReplicationInterestDescriptorBase
{
	GENERATED_BODY()
}; // struct FFlecsReplicationInterestDescriptorBase

USTRUCT()
struct UNREALFLECS_API FFlecsReplicationEveryoneInterestDescriptor : public FFlecsReplicationInterestDescriptorBase
{
	GENERATED_BODY()
}; // struct FFlecsReplicationEveryoneInterestDescriptor

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsReplicationOwnerInterestDescriptor : public FFlecsReplicationInterestDescriptorBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Networking")
	FFlecsReplicationConnectionId OwnerConnection;
}; // struct FFlecsReplicationOwnerInterestDescriptor

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsReplicationSpatialCellInterestDescriptor : public FFlecsReplicationInterestDescriptorBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Networking")
	FIntVector Cell = FIntVector::ZeroValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Networking", meta = (ClampMin = "0.0001"))
	float CellSize = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Networking")
	int32 SpatialLayer = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Networking", meta = (ClampMin = "0.0"))
	float BubbleRadius = 0.0f;
}; // struct FFlecsReplicationSpatialCellInterestDescriptor

struct UNREALFLECS_API FFlecsReplicationInterestPolicyNames
{
	static const FName Everyone;
	static const FName Owner;
	static const FName SpatialCell;
}; // struct FFlecsReplicationInterestPolicyNames

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsReplicationInterestBinding
{
	GENERATED_BODY()

	template <typename TDescriptor>
	static FFlecsReplicationInterestBinding Make(const FName InPolicyName, const TDescriptor& InDescriptor)
	{
		FFlecsReplicationInterestBinding Result;
		Result.PolicyName = InPolicyName;
		Result.Descriptor.InitializeAs<TDescriptor>(InDescriptor);
		return Result;
	}

	static FFlecsReplicationInterestBinding Everyone();

	friend bool operator==(const FFlecsReplicationInterestBinding&, const FFlecsReplicationInterestBinding&) = default;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Networking")
	FName PolicyName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Networking")
	TInstancedStruct<FFlecsReplicationInterestDescriptorBase> Descriptor;
}; // struct FFlecsReplicationInterestBinding

/** Complete logical routing, filtering, cadence, priority, and page-capacity description. */
USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsReplicationRouteDescriptor
{
	GENERATED_BODY()

	FFlecsReplicationRouteDescriptor();
	static FFlecsReplicationRouteDescriptor Default();

	friend bool operator==(const FFlecsReplicationRouteDescriptor&, const FFlecsReplicationRouteDescriptor&) = default;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Networking")
	FFlecsReplicationRouteKey LogicalKey = FFlecsReplicationRouteKey::Default();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Networking")
	FFlecsReplicationInterestBinding Interest;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Networking", meta = (ClampMin = "0.0"))
	float PollFrequency = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Networking", meta = (ClampMin = "0.0"))
	float StaticPriority = 1.0f;

	/** Zero inherits UFlecsNetworkingModuleSettings::DefaultPageEntityLimit. */
	UPROPERTY(EditAnywhere, Category = "Flecs | Networking", meta = (ClampMin = "0"))
	uint16 PageEntityLimit = 0;

	/** Zero inherits UFlecsNetworkingModuleSettings::DefaultPageRetainedPayloadByteLimit. */
	UPROPERTY(EditAnywhere, Category = "Flecs | Networking", meta = (ClampMin = "0"))
	uint32 PageRetainedPayloadByteLimit = 0;

}; // struct FFlecsReplicationRouteDescriptor

/** Transport-neutral, unmodified connection viewpoints used by policies. */
struct UNREALFLECS_API FFlecsReplicationConnectionView
{
	TArray<FVector> Positions;
	TArray<FVector> Directions;
	TArray<float> FieldOfViewRadians;
}; // struct FFlecsReplicationConnectionView

/** Heterogeneous policy-owned connection fragments. */
struct UNREALFLECS_API FFlecsReplicationConnectionInterestContext
{
	template <typename TFragment>
	void Set(const TFragment& InFragment)
	{
		Fragments.Add(TFragment::StaticStruct(), FInstancedStruct::Make<TFragment>(InFragment));
	}

	template <typename TFragment>
	bool Remove()
	{
		return Fragments.Remove(TFragment::StaticStruct()) > 0;
	}

	template <typename TFragment>
	NO_DISCARD const TFragment* Find() const
	{
		const FInstancedStruct* Fragment = Fragments.Find(TFragment::StaticStruct());
		return Fragment ? Fragment->GetPtr<TFragment>() : nullptr;
	}

	NO_DISCARD bool IsEmpty() const
	{
		return Fragments.IsEmpty();
	}

private:
	TMap<const UScriptStruct*, FInstancedStruct> Fragments;
}; // struct FFlecsReplicationConnectionInterestContext

/** Deterministically maps a world position to a 3D cell, including negative coordinates. */
UNREALFLECS_API FIntVector FlecsReplicationSpatialCell(const FVector& Position, float CellSize);

/** Builds the standard spatial-cell route from game-owned position data. */
UNREALFLECS_API FFlecsReplicationRouteDescriptor MakeFlecsSpatialCellRoute(const FVector& Position,
	float CellSize, int32 SpatialLayer, float BubbleRadius, FName LogicalRoute = TEXT("Spatial"));

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
 * One complete baseline or same-layout component-key delta for an entity.
 *
 * CompositionRevision changes only when LayoutId changes. StateRevision is
 * monotonically increased by the authority for every published update and is
 * used by clients to discard stale delivery.
 */
UENUM()
enum class EFlecsReplicatedEntityUpdateKind : uint8
{
	Full,
	Delta
}; // enum class EFlecsReplicatedEntityUpdateKind

USTRUCT()
struct UNREALFLECS_API FFlecsReplicatedEntityUpdate
{
	GENERATED_BODY()

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

	/** Layout-key indices whose values are carried by this revision. */
	UPROPERTY()
	TArray<uint16> ChangedKeys;

	UPROPERTY()
	TArray<FFlecsReplicatedValue> Values;

	NO_DISCARD uint32 GetPayloadByteCount() const;
	NO_DISCARD bool IsKeyChanged(const uint16 KeyIndex) const;
	
}; // struct FFlecsReplicatedEntityUpdate

/** Kinds of transport-to-core records accepted by the client inbox. */
UENUM()
enum class EFlecsReplicationInboxRecordType : uint8
{
	Layout,
	UpsertEntity,
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
	FFlecsNetworkId NetworkId;
}; // struct FFlecsReplicationInboxRecord

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
		const FFlecsEntityHandle& Entity, bool& bOutWasCreated, OUT FString& OutError);
	
	/** Finds a previously generated or accepted layout definition. */
	NO_DISCARD const FFlecsReplicationLayoutDefinition* Find(FFlecsReplicationLayoutId Id) const;
	
	/** Adds an already validated remote layout, rejecting identity collisions. */
	bool AddRemoteDefinition(const FFlecsReplicationLayoutDefinition& Definition, OUT FString& OutError);
	
	void HandleTableDestruction(const TSolidNotNull<const UFlecsWorld*> World);

private:
	// @TODO: Handle Table destruction and remove from cache.
	TMap<const flecs::table_t*, FFlecsReplicationLayoutId> TableCache;
	TMap<FFlecsReplicationLayoutId, FFlecsReplicationLayoutDefinition> Definitions;
	
}; // class FFlecsReplicationLayoutRegistry
