// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Entities/FlecsEntityHandle.h"

#include "FlecsReplicationProfileParamsBase.generated.h"

USTRUCT(BlueprintInternalUseOnly)
struct FFlecsReplicationProfileParamsBase
{
	GENERATED_BODY()
	
public:
	FFlecsReplicationProfileParamsBase() = default;
	virtual ~FFlecsReplicationProfileParamsBase() = default;

	virtual void ApplyToEntity(const FFlecsEntityHandle& InEntity) const
		PURE_VIRTUAL(FFlecsReplicationProfileParamsBase:ApplyToEntity, )
	
}; // struct FFlecsReplicationProfileParamsBase

template <>
struct TStructOpsTypeTraits<FFlecsReplicationProfileParamsBase> : public TStructOpsTypeTraitsBase2<FFlecsReplicationProfileParamsBase>
{
	enum
	{
		WithPureVirtual = true,
	}; // enum
	
}; // struct TStructOpsTypeTraits<FFlecsReplicationProfileParamsBase>