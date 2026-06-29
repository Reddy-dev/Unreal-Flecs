// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "flecs.h"

#include "CoreMinimal.h"

#include "StructUtils/InstancedStruct.h"

#include "Types/SolidNotNull.h"

#include "Entities/FlecsId.h"
#include "Entities/FlecsTableHandle.h"

#include "FlecsGroupByCallbackDefinition.generated.h"

struct FFlecsGroupByCallbackDefinition;

namespace UE::Flecs::Queries
{
	using FGroupByFunctionType = uint64(*)(const TSolidNotNull<UFlecsWorldInterfaceObject*>, FFlecsTableHandle, FFlecsId, void* /*Context*/);
	using FGroupByCreateGroupFunctionType = void*(*)(const TSolidNotNull<UFlecsWorldInterfaceObject*>, uint64 /*GroupId*/, void* /*GroupByContext*/);
	using FGroupByDeleteGroupFunctionType = void(*)(const TSolidNotNull<UFlecsWorldInterfaceObject*>, uint64 /*GroupId*/, void* /*GroupContext*/, void* /*GroupByContext*/);
	
	struct FFlecsGroupByContextData
	{
	public:
		TInstancedStruct<FFlecsGroupByCallbackDefinition> Definition;
		void* UserContext = nullptr;
		
	}; // struct FFlecsGroupByContextData
	
} // namespace UE::Flecs::Queries

USTRUCT(BlueprintInternalUseOnly)
struct UNREALFLECS_API FFlecsGroupByCallbackDefinition
{
	GENERATED_BODY()
	
public:
	FORCEINLINE FFlecsGroupByCallbackDefinition() = default;
	
	virtual ~FFlecsGroupByCallbackDefinition() = default;
	
	virtual UE::Flecs::Queries::FGroupByFunctionType GetGroupByFunction() const
	{
		return GroupByFunctionPtr;
	}
	
	virtual UE::Flecs::Queries::FGroupByCreateGroupFunctionType GetCreateGroupFunction() const
	{
		return CreateGroupFunctionPtr;
	}
	
	virtual UE::Flecs::Queries::FGroupByDeleteGroupFunctionType GetDeleteGroupFunction() const
	{
		return DeleteGroupFunctionPtr;
	}
	
	UE::Flecs::Queries::FGroupByFunctionType GroupByFunctionPtr = nullptr;
	UE::Flecs::Queries::FGroupByCreateGroupFunctionType CreateGroupFunctionPtr = nullptr;
	UE::Flecs::Queries::FGroupByDeleteGroupFunctionType DeleteGroupFunctionPtr = nullptr;
	
}; // struct FFlecsGroupByCallbackDefinition

template <>
struct TStructOpsTypeTraits<FFlecsGroupByCallbackDefinition> : public TStructOpsTypeTraitsBase2<FFlecsGroupByCallbackDefinition>
{
	enum
	{
		WithCopy = true
	}; // enum
	
}; // struct TStructOpsTypeTraits<FFlecsGroupByCallbackDefinition>
