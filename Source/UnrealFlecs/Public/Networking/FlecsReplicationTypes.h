// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Queue.h"
#include "Networking/FlecsComponentReplicationDescriptor.h"
#include "Networking/FlecsNetworkId.h"

#include "FlecsReplicationTypes.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsReplicationLayoutId
{
	GENERATED_BODY()

	FFlecsReplicationLayoutId() = default;
	explicit FFlecsReplicationLayoutId(const FGuid& InValue) : Value(InValue) {}
	NO_DISCARD bool IsValid() const { return Value.IsValid(); }
	NO_DISCARD FString ToString() const { return Value.ToString(EGuidFormats::DigitsWithHyphensLower); }
	friend bool operator==(const FFlecsReplicationLayoutId&, const FFlecsReplicationLayoutId&) = default;
	friend uint32 GetTypeHash(const FFlecsReplicationLayoutId& Id) { return GetTypeHash(Id.Value); }

	UPROPERTY()
	FGuid Value;
};

UENUM()
enum class EFlecsReplicationKeyKind : uint8
{
	Component,
	Pair
};

UENUM()
enum class EFlecsReplicationPairTargetKind : uint8
{
	None,
	Schema,
	StableValue,
	Entity
};

/** Stable, transport-safe representation of an ordinary ID or pair. */
USTRUCT()
struct UNREALFLECS_API FFlecsReplicationKey
{
	GENERATED_BODY()

	UPROPERTY()
	EFlecsReplicationKeyKind Kind = EFlecsReplicationKeyKind::Component;

	UPROPERTY()
	FFlecsReplicationSchemaId RelationshipSchema;

	UPROPERTY()
	uint32 RelationshipVersion = 0;

	UPROPERTY()
	FFlecsReplicationSchemaId StorageSchema;

	UPROPERTY()
	uint32 StorageVersion = 0;

	UPROPERTY()
	EFlecsReplicationPairTargetKind TargetKind = EFlecsReplicationPairTargetKind::None;

	UPROPERTY()
	FFlecsReplicationSchemaId TargetSchema;

	UPROPERTY()
	uint32 TargetVersion = 0;

	UPROPERTY()
	FString StableTargetName;

	UPROPERTY()
	FFlecsNetworkId EntityTarget;

	UPROPERTY()
	bool bHasPayload = false;

	NO_DISCARD FString CanonicalString() const;
	friend bool operator==(const FFlecsReplicationKey&, const FFlecsReplicationKey&) = default;
};

USTRUCT()
struct UNREALFLECS_API FFlecsReplicationLayoutDefinition
{
	GENERATED_BODY()

	UPROPERTY()
	FFlecsReplicationLayoutId LayoutId;

	UPROPERTY()
	TArray<FFlecsReplicationKey> Keys;
};

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsReplicationRouteKey
{
	GENERATED_BODY()

	FFlecsReplicationRouteKey() = default;
	explicit FFlecsReplicationRouteKey(const FName InName) : Name(InName) {}
	static FFlecsReplicationRouteKey Default() { return FFlecsReplicationRouteKey(TEXT("Default")); }
	friend bool operator==(const FFlecsReplicationRouteKey&, const FFlecsReplicationRouteKey&) = default;
	friend uint32 GetTypeHash(const FFlecsReplicationRouteKey& Key) { return GetTypeHash(Key.Name); }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Networking")
	FName Name = TEXT("Default");
};

USTRUCT()
struct UNREALFLECS_API FFlecsReplicatedValue
{
	GENERATED_BODY()

	UPROPERTY()
	uint16 KeyIndex = 0;

	UPROPERTY()
	TArray<uint8> Bytes;
};

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

UENUM()
enum class EFlecsReplicationInboxRecordType : uint8
{
	Layout,
	UpsertEntity,
	RemoveEntity,
	DetachShard
};

struct UNREALFLECS_API FFlecsReplicationInboxRecord
{
	EFlecsReplicationInboxRecordType Type = EFlecsReplicationInboxRecordType::Layout;
	FGuid SourceShard;
	FFlecsReplicationLayoutDefinition Layout;
	FFlecsReplicatedEntitySnapshot Snapshot;
	FFlecsNetworkId NetworkId;
};

class UNREALFLECS_API FFlecsReplicationInbox
{
public:
	void Enqueue(FFlecsReplicationInboxRecord Record) { Records.Enqueue(MoveTemp(Record)); }
	bool Dequeue(FFlecsReplicationInboxRecord& OutRecord) { return Records.Dequeue(OutRecord); }
	NO_DISCARD bool IsEmpty() const { return Records.IsEmpty(); }

private:
	TQueue<FFlecsReplicationInboxRecord, EQueueMode::Mpsc> Records;
};

class UNREALFLECS_API FFlecsReplicationLayoutRegistry
{
public:
	static FFlecsReplicationLayoutId ComputeLayoutId(const TArray<FFlecsReplicationKey>& Keys);
	const FFlecsReplicationLayoutDefinition* BuildForEntity(const UFlecsWorld* World,
		const FFlecsEntityHandle& Entity, bool& bOutWasCreated, FString& OutError);
	NO_DISCARD const FFlecsReplicationLayoutDefinition* Find(FFlecsReplicationLayoutId Id) const;
	bool AddRemoteDefinition(const FFlecsReplicationLayoutDefinition& Definition, FString& OutError);

private:
	TMap<const flecs::table_t*, FFlecsReplicationLayoutId> TableCache;
	TMap<FFlecsReplicationLayoutId, FFlecsReplicationLayoutDefinition> Definitions;
};
