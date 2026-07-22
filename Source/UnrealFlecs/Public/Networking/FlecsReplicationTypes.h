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

USTRUCT()
struct UNREALFLECS_API FFlecsEntityReplicationSnapshot
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	FFlecsReplicationLayoutId LayoutId;

	UPROPERTY()
	TArray<FFlecsReplicatedValue> Values;
	
}; // struct FFlecsEntityReplicationSnapshot

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

private:
	// @TODO: Handle Table destruction and remove from cache.
	TMap<const flecs::table_t*, FFlecsReplicationLayoutId> TableCache;
	TMap<FFlecsReplicationLayoutId, FFlecsReplicationLayoutDefinition> Definitions;
	
}; // class FFlecsReplicationLayoutRegistry
