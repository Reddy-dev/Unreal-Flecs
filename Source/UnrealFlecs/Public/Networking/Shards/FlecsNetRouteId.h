// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SolidMacros/Macros.h"

#include "FlecsNetRouteId.generated.h"

/** Stable bridge-local identity used to resolve a replication shard. */
USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsNetRouteId
{
	GENERATED_BODY()

public:
	FFlecsNetRouteId() = default;

	FORCEINLINE explicit FFlecsNetRouteId(const FName InName)
		: Name(InName)
	{
	}

	static NO_DISCARD FORCEINLINE FFlecsNetRouteId Default()
	{
		return FFlecsNetRouteId(FName("Default"));
	}

	NO_DISCARD FORCEINLINE bool IsValid() const
	{
		return !Name.IsNone();
	}

	friend bool operator==(const FFlecsNetRouteId&, const FFlecsNetRouteId&) = default;

	friend uint32 GetTypeHash(const FFlecsNetRouteId& InRouteId)
	{
		return GetTypeHash(InRouteId.Name);
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Networking")
	FName Name = FName("Default");

}; // struct FFlecsNetRouteId
