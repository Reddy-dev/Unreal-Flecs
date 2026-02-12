// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "flecs.h"

#include "CoreMinimal.h"

#include "StructUtils/InstancedStruct.h"

#include "SolidMacros/Macros.h"
#include "Types/SolidNotNull.h"

#include "Entities/FlecsId.h"
#include "Entities/FlecsTableHandle.h"

#include "FlecsGroupByCallbackDefinition.generated.h"

namespace Unreal::Flecs::Queries
{
	using FUnrealGroupByFunctionType = uint64(*)(const TSolidNotNull<UFlecsWorld*>, FFlecsTableHandle, FFlecsId, FInstancedStruct);
	
	struct FFlecsGroupByContextData
	{
	public:
		
	}; // struct FFlecsGroupByContextData
	
} // namespace Unreal::Flecs::Queries

// @TODO: Not Implemented
USTRUCT(BlueprintInternalUseOnly)
struct UNREALFLECS_API FFlecsGroupByCallbackDefinition
{
	GENERATED_BODY()
	
public:
	FORCEINLINE FFlecsGroupByCallbackDefinition() = default;
	virtual ~FFlecsGroupByCallbackDefinition() = default;
	
	virtual Unreal::Flecs::Queries::FUnrealGroupByFunctionType GetGroupByFunction() const PURE_VIRTUAL(FFlecsGroupByCallbackDefinition, return nullptr;);
	virtual UScriptStruct* GetGroupByContextStruct() const { return nullptr; }
	
}; // struct FFlecsGroupByCallbackDefinition

template <>
struct TStructOpsTypeTraits<FFlecsGroupByCallbackDefinition> : public TStructOpsTypeTraitsBase2<FFlecsGroupByCallbackDefinition>
{
	enum
	{
		WithPureVirtual = true
	}; // enum
	
}; // struct TStructOpsTypeTraits<FFlecsGroupByCallbackDefinition>

