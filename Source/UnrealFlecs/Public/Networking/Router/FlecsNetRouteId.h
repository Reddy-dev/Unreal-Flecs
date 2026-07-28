// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SolidMacros/Macros.h"

#include "Properties/FlecsComponentProperties.h"

#include "FlecsNetRouteId.generated.h"

/** Stable bridge-local identity used to resolve a replication shard. */
USTRUCT()
struct UNREALFLECS_API FFlecsNetRouteId
{
	GENERATED_BODY()
	
	static constexpr flecs::on_instantiate on_instantiate = flecs::on_instantiate::inherit;

public:
	FFlecsNetRouteId() = default;

	FORCEINLINE explicit FFlecsNetRouteId(const FName& InName)
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

	UPROPERTY()
	FName Name = FName("Default");

}; // struct FFlecsNetRouteId

template <>
struct TFlecsComponentTraits<FFlecsNetRouteId> : public TFlecsComponentTraitsBase<FFlecsNetRouteId>
{
}; // struct TFlecsComponentTraits<FFlecsNetRouteId>
