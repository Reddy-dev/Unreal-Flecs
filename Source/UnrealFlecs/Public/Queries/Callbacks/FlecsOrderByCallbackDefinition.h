// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SolidMacros/Macros.h"

#include "Entities/FlecsId.h"

#include "FlecsOrderByCallbackDefinition.generated.h"

namespace Unreal::Flecs::Queries
{
	using FOrderByFunctionType = int32(*)(FFlecsId, const void*, FFlecsId, const void*);
	
	template <typename T>
	using TOrderByFunction = int32(*)(FFlecsId, const T*, FFlecsId, const T*);
	
	MSVC_WARNING_PUSH
	MSVC_WARNING_DISABLE(4191) // C4191: unsafe conversion from function pointer to data pointer
	
	NO_DISCARD FORCEINLINE ecs_order_by_action_t MakeOrderByFunction(FOrderByFunctionType InFunction)
	{
		return reinterpret_cast<ecs_order_by_action_t>(InFunction);
	}
	
	template <typename T>
	NO_DISCARD FORCEINLINE FOrderByFunctionType ConvertToUntypedOrderByFunction(TOrderByFunction<T> InTypedFunction)
	{
		return reinterpret_cast<FOrderByFunctionType>(InTypedFunction);
	}
	
	MSVC_WARNING_POP
	
} // namespace Unreal::Flecs::Queries

USTRUCT(BlueprintInternalUseOnly)
struct UNREALFLECS_API FFlecsOrderByCallbackDefinition
{
	GENERATED_BODY()
	
public:
	FORCEINLINE FFlecsOrderByCallbackDefinition() = default;
	virtual ~FFlecsOrderByCallbackDefinition() = default;
	
	virtual Unreal::Flecs::Queries::FOrderByFunctionType GetOrderByFunction() const PURE_VIRTUAL(FFlecsOrderByCallbackDefinition, return nullptr;);
	
}; // struct FFlecsOrderByCallbackDefinition

template <>
struct TStructOpsTypeTraits<FFlecsOrderByCallbackDefinition> : public TStructOpsTypeTraitsBase2<FFlecsOrderByCallbackDefinition>
{
	enum
	{
		WithPureVirtual = true
	}; // enum
	
}; // struct TStructOpsTypeTraits<FFlecsOrderByCallbackDefinition>