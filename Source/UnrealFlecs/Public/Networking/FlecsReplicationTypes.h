// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Containers/Queue.h"
#include "StructUtils/InstancedStruct.h"

#include "Networking/FlecsComponentReplicationDescriptor.h"
#include "Layout/FlecsReplicationLayoutId.h"

#include "FlecsReplicationTypes.generated.h"

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
