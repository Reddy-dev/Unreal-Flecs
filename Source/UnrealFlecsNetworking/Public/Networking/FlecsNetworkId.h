// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once


#include "Properties/FlecsComponentProperties.h"

#include "Networking/FlecsReplicatedEntityComponent.h"

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
struct UNREALFLECSNETWORKING_API FFlecsNetworkId
{
	GENERATED_BODY()
	
	static constexpr uint64 InvalidValue = 0ull;
	
	static constexpr uint64 SlotBitCount = 32ull;
	static constexpr uint64 GenerationBitCount = 32ull;

	static constexpr uint64 SlotMask = (1ull << SlotBitCount) - 1ull;
	
	static constexpr uint64 GenerationValueMask = (1ull << GenerationBitCount) - 1ull;
	static constexpr uint64 GenerationMask = GenerationValueMask << SlotBitCount;
	
public:

	FFlecsNetworkId() = default;
	
	explicit constexpr FFlecsNetworkId(const uint64 InValue) : Value(InValue) {}
	
	constexpr FFlecsNetworkId(const uint32 InSlot, const uint32 InGeneration)
		: Value((static_cast<uint64>(InSlot) & SlotMask)
			| ((static_cast<uint64>(InGeneration) & GenerationValueMask) << SlotBitCount))
	{
	}

	NO_DISCARD constexpr bool IsValid() const
	{
		return Value != 0;
	}
	
	NO_DISCARD constexpr uint64 GetValue() const
	{
		return Value;
	}
	
	NO_DISCARD constexpr uint32 GetSlot() const
	{
		return static_cast<uint32>(Value & SlotMask);
	}
	
	NO_DISCARD constexpr uint32 GetGeneration() const
	{
		return static_cast<uint32>((Value & GenerationMask) >> SlotBitCount);
	}
	
	NO_DISCARD FORCEINLINE FString ToString() const
	{
		return FString::Printf(TEXT("Slot:%u Gen:%u"), GetSlot(), GetGeneration());
	}

	NO_DISCARD constexpr bool operator==(const FFlecsNetworkId& Other) const
	{
		return Value == Other.Value;
	}
	
	NO_DISCARD friend constexpr bool operator<(const FFlecsNetworkId& A, const FFlecsNetworkId& B)
	{
		return A.Value < B.Value;
	}
	
	NO_DISCARD friend uint32 GetTypeHash(const FFlecsNetworkId& InId)
	{
		return GetTypeHash(InId.Value);
	}

	UPROPERTY()
	uint64 Value = 0;
};

static_assert(sizeof(FFlecsNetworkId) == sizeof(uint64));

template <>
struct TFlecsComponentTraits<FFlecsNetworkId> : public TFlecsComponentTraitsBase<FFlecsNetworkId>
{
	using WithTypes = TTuple<FFlecsReplicatedEntityComponent>;
}; // struct TFlecsComponentTraits<FFlecsNetworkId>

template<>
struct TStructOpsTypeTraits<FFlecsNetworkId> : public TStructOpsTypeTraitsBase2<FFlecsNetworkId>
{
	enum
	{
		WithIdenticalViaEquality = true
	};
	
}; // struct TStructOpsTypeTraits<FFlecsNetworkId>
