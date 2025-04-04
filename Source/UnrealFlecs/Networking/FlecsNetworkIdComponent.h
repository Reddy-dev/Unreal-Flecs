﻿// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SolidMacros/Macros.h"
#include "FlecsNetworkIdComponent.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsNetworkIdComponent
{
	GENERATED_BODY()

	NO_DISCARD FORCEINLINE friend uint32 GetTypeHash(const FFlecsNetworkIdComponent& InComponent)
	{
		return GetTypeHash(InComponent.GetNetworkId());
	}

public:
	FORCEINLINE FFlecsNetworkIdComponent() = default;
	FORCEINLINE FFlecsNetworkIdComponent(const uint32 InNetworkId) : NetworkId(InNetworkId) {}

	NO_DISCARD FORCEINLINE uint32 GetNetworkId() const
	{
		return NetworkId.Get(INDEX_NONE);
	}
	
	FORCEINLINE void SetNetworkId(const uint32 InNetworkId)
	{
		NetworkId = InNetworkId;
	}

	NO_DISCARD FORCEINLINE bool IsValid() const
	{
		return NetworkId.IsSet();
	}

	FORCEINLINE void Reset()
	{
		NetworkId.Reset();
	}

	FORCEINLINE bool operator==(const FFlecsNetworkIdComponent& Other) const
	{
		return NetworkId == Other.NetworkId;
	}

	FORCEINLINE bool operator!=(const FFlecsNetworkIdComponent& Other) const
	{
		return !(*this == Other);
	}

	FORCEINLINE bool operator==(const uint32 Other) const
	{
		return NetworkId == Other;
	}

	FORCEINLINE bool operator!=(const uint32 Other) const
	{
		return !(*this == Other);
	}
	
	UPROPERTY()
	TOptional<uint32> NetworkId;

	NO_DISCARD FORCEINLINE FString ToString() const
	{
		return FString::Printf(TEXT("NetworkId: %lu"), NetworkId.Get(INDEX_NONE));
	}

	FORCEINLINE bool NetSerialize(FArchive& Ar, MAYBE_UNUSED UPackageMap* Map, bool& bOutSuccess)
	{
		SerializeOptionalValue<TOptional<uint32>>(Ar.IsSaving(), Ar,
			NetworkId, TOptional<uint32>());

		bOutSuccess = true;
		return true;
	}
	
}; // struct FNetworkIdComponent

template<>
struct TStructOpsTypeTraits<FFlecsNetworkIdComponent> : public TStructOpsTypeTraitsBase2<FFlecsNetworkIdComponent>
{
	enum
	{
		WithNetSerializer = true,
	}; // enum
}; // struct TStructOpsTypeTraits<FFlecsNetworkIdComponent>
