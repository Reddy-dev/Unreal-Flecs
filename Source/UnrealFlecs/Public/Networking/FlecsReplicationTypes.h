// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Containers/Queue.h"
#include "StructUtils/InstancedStruct.h"

#include "SolidMacros/Macros.h"

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
