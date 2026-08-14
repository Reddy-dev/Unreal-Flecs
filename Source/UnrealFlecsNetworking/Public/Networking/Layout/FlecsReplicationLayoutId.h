// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SolidMacros/Macros.h"

#include "FlecsReplicationLayoutId.generated.h"

/** Deterministic identity of a complete replicated component/pair composition. */
USTRUCT(BlueprintType)
struct UNREALFLECSNETWORKING_API FFlecsReplicationLayoutId
{
	GENERATED_BODY()
	
	friend uint32 GetTypeHash(const FFlecsReplicationLayoutId& Id)
	{
		return GetTypeHash(Id.Value);
	}
	
public:

	FFlecsReplicationLayoutId() = default;
	explicit FFlecsReplicationLayoutId(const FGuid& InValue) 
		: Value(InValue)
	{
	}
	
	NO_DISCARD bool IsValid() const
	{
		return Value.IsValid();
	}
	
	NO_DISCARD FString ToString() const
	{
		return Value.ToString(EGuidFormats::DigitsWithHyphensLower);
	}
	
	friend bool operator==(const FFlecsReplicationLayoutId&, const FFlecsReplicationLayoutId&) = default;

	UPROPERTY()
	FGuid Value;
	
}; // struct FFlecsReplicationLayoutId