// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Net/NetConnectionIdHandler.h"
#include "SolidMacros/Macros.h"

#include "FlecsReplicationConnectionId.generated.h"

USTRUCT()
struct UNREALFLECS_API FFlecsReplicationConnectionId
{
	GENERATED_BODY()
	
	static constexpr uint32 InvalidConnectionId = 0;
	
public:
	FFlecsReplicationConnectionId() = default;

	explicit constexpr FFlecsReplicationConnectionId(const uint32 InConnectionId)
			: ConnectionId(InConnectionId)
	{
	}

	NO_DISCARD constexpr bool IsValid() const
	{
		return ConnectionId != InvalidConnectionId;
	}

	NO_DISCARD constexpr uint32 GetConnectionId() const
	{
		return ConnectionId;
	}

	NO_DISCARD FORCEINLINE FString ToString() const
	{
		return FString::Printf(TEXT("Connection:%u"), ConnectionId);
	}

	NO_DISCARD constexpr bool operator==(const FFlecsReplicationConnectionId Other) const
	{
		return ConnectionId == Other.ConnectionId;
	}

	NO_DISCARD friend constexpr bool operator<(const FFlecsReplicationConnectionId A,
			const FFlecsReplicationConnectionId B)
	{
		return A.ConnectionId < B.ConnectionId;
	}

	NO_DISCARD friend uint32 GetTypeHash(const FFlecsReplicationConnectionId& InConnectionId)
	{
		return GetTypeHash(InConnectionId.ConnectionId);
	}
	
private:
	UPROPERTY()
	uint32 ConnectionId = InvalidConnectionId;
}; // struct FFlecsReplicationConnectionId

static_assert(sizeof(FFlecsReplicationConnectionId) <= sizeof(uint32) && alignof(FFlecsReplicationConnectionId) == alignof(uint32), 
	"FFlecsReplicationConnectionId must be able to fit in a uint32 for NetConnectionIdHandler");
static_assert(std::is_trivially_copyable_v<FFlecsReplicationConnectionId>, 
	"FFlecsReplicationConnectionId must be trivially copyable for NetConnectionIdHandler");