// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SolidMacros/Macros.h"

#include "FlecsComponentReplicationDescriptor.h"
#include "FlecsNetworkId.h"

#include "FlecsReplicationKey.generated.h"


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
	
	NO_DISCARD const FFlecsComponentReplicationDescriptor* TryGetDescriptor(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld) const;
	
	static NO_DISCARD TValueOrError<FFlecsReplicationIndividualKey, FString> 
		BuildIndividualKey(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld, const FFlecsId InId);
	
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
	
	static NO_DISCARD EFlecsReplicationKeyStorageKind GetStorageKindForPair(
		const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld, 
		const FFlecsId InPairId)
	{
		const FFlecsId FirstId = InPairId.GetFirst();
		const FFlecsId SecondId = InPairId.GetSecond();
		
		return GetStorageKindForPair(InWorld, FirstId, SecondId);
	}
	
	static NO_DISCARD EFlecsReplicationKeyStorageKind GetStorageKindForPair(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld, 
		const FFlecsId InFirstId, const FFlecsId InSecondId);


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
	
	NO_DISCARD const FFlecsComponentReplicationDescriptor* TryGetStorageDescriptor(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld) const;
	
}; // struct FFlecsReplicationKey