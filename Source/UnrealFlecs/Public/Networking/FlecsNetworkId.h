// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Networking/FlecsReplicatedEntityComponent.h"
#include "Properties/FlecsComponentProperties.h"

#include "FlecsNetworkId.generated.h"

/**
 * A session-scoped replicated-entity identity.
 *
 * The low 32 bits are a reusable slot, followed by a generation and a session
 * epoch. The complete value, rather than the slot alone, identifies a remote
 * entity and prevents a reused slot from being mistaken for an older entity.
 * Zero, and every value with a zero epoch, is invalid.
 */
USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsNetworkId
{
	GENERATED_BODY()

	static constexpr uint64 SlotMask = 0x00000000FFFFFFFFull;
	static constexpr uint64 GenerationMask = 0x0000FFFF00000000ull;
	static constexpr uint64 EpochMask = 0xFFFF000000000000ull;

	FFlecsNetworkId() = default;
	explicit constexpr FFlecsNetworkId(const uint64 InValue) : Value(InValue) {}
	constexpr FFlecsNetworkId(const uint32 InSlot, const uint16 InGeneration, const uint16 InSessionEpoch)
		: Value(static_cast<uint64>(InSlot)
			| (static_cast<uint64>(InGeneration) << 32u)
			| (static_cast<uint64>(InSessionEpoch) << 48u))
	{
	}

	NO_DISCARD constexpr bool IsValid() const { return Value != 0 && GetSessionEpoch() != 0; }
	NO_DISCARD constexpr uint64 GetValue() const { return Value; }
	NO_DISCARD constexpr uint32 GetSlot() const { return static_cast<uint32>(Value & SlotMask); }
	NO_DISCARD constexpr uint16 GetGeneration() const { return static_cast<uint16>((Value & GenerationMask) >> 32u); }
	NO_DISCARD constexpr uint16 GetSessionEpoch() const { return static_cast<uint16>((Value & EpochMask) >> 48u); }

	NO_DISCARD constexpr bool operator==(const FFlecsNetworkId& Other) const
	{
		return Value == Other.Value;
	}
	
	NO_DISCARD friend constexpr bool operator<(const FFlecsNetworkId& A, const FFlecsNetworkId& B) { return A.Value < B.Value; }
	NO_DISCARD friend uint32 GetTypeHash(const FFlecsNetworkId& InId) { return GetTypeHash(InId.Value); }

	UPROPERTY()
	uint64 Value = 0;
};

static_assert(sizeof(FFlecsNetworkId) == sizeof(uint64));

/** Serializable value wrapper for references to another replicated Flecs entity. */
USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsReplicatedEntityReference
{
	GENERATED_BODY()

	NO_DISCARD bool IsSet() const { return NetworkId.IsValid(); }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Networking")
	FFlecsNetworkId NetworkId;
};

/**
 * Authority-side allocator for FFlecsNetworkId values.
 *
 * Released slots are reused with an incremented generation. Resetting begins a
 * new session epoch and invalidates identities from the previous session.
 */
class UNREALFLECS_API FFlecsNetworkIdAllocator
{
public:
	explicit FFlecsNetworkIdAllocator(uint16 InSessionEpoch = 1);

	/** Allocates a new valid identity, or an invalid identity if the slot space is exhausted. */
	FFlecsNetworkId Allocate();
	/** Releases a currently allocated identity; stale generations are rejected. */
	bool Release(FFlecsNetworkId InId);
	/** Clears allocations and selects the epoch used by subsequent identities. */
	void Reset(uint16 InSessionEpoch);

	NO_DISCARD uint16 GetSessionEpoch() const { return SessionEpoch; }

private:
	uint16 SessionEpoch = 1;
	uint32 NextSlot = 1;
	TArray<uint32> FreeSlots;
	TMap<uint32, uint16> SlotGenerations;
	TSet<uint32> AllocatedSlots;
};

template <>
struct TFlecsComponentTraits<FFlecsNetworkId> : public TFlecsComponentTraitsBase<FFlecsNetworkId>
{
	using WithTypes = TTuple<FFlecsReplicatedEntityComponent>;
};

template<>
struct TStructOpsTypeTraits<FFlecsNetworkId> : public TStructOpsTypeTraitsBase2<FFlecsNetworkId>
{
	enum { WithIdenticalViaEquality = true };
};
