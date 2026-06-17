// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "flecs.h"

#include "CoreMinimal.h"

#include "SolidMacros/Macros.h"

#include "FlecsEntityHandle.h"

#include "FlecsComponentRefTypes.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsUntypedComponentRef
{
	GENERATED_BODY()
	
public:
	FORCEINLINE FFlecsUntypedComponentRef() = default;
	
	FORCEINLINE explicit FFlecsUntypedComponentRef(const flecs::untyped_ref& InFlecsRef)
		: InternalFlecsReference(InFlecsRef)
	{
	}
	
	FORCEINLINE explicit FFlecsUntypedComponentRef(const FFlecsEntityHandle& Entity, const FFlecsId ComponentId)
		: InternalFlecsReference(Entity, ComponentId)
	{
	}
	
	NO_DISCARD FORCEINLINE FFlecsEntityHandle GetEntity() const
	{
		return FFlecsEntityHandle(InternalFlecsReference.entity());
	}
	
	NO_DISCARD FORCEINLINE FFlecsId GetComponentId() const
	{
		return FFlecsId(InternalFlecsReference.component());
	}
	
	template <bool bThreadSafe = true>
	requires (bThreadSafe)
	NO_DISCARD FORCEINLINE bool IsValid()
	{
		return InternalFlecsReference.has();
	}
	
	template <bool bThreadSafe>
	requires (!bThreadSafe)
	NO_DISCARD FORCEINLINE bool IsValid() const
	{
		return const_cast<flecs::untyped_ref&>(InternalFlecsReference).has();
	}
	
	FORCEINLINE operator bool()
	{
		return IsValid();
	}
	
	NO_DISCARD FORCEINLINE void* GetComponentValue()
	{
		return InternalFlecsReference.get();
	}
	
	NO_DISCARD FORCEINLINE void* TryGetComponentValue()
	{
		return InternalFlecsReference.try_get();
	}

private:
	flecs::untyped_ref InternalFlecsReference;
	
}; // struct FFlecsUntypedComponentRef

template <typename T>
struct TFlecsComponentRef : public FFlecsUntypedComponentRef
{
	using FFlecsUntypedComponentRef::FFlecsUntypedComponentRef;
public:
	FORCEINLINE TFlecsComponentRef() = default;
	
	FORCEINLINE T* operator->()
	{
		return static_cast<T*>(GetComponentValue());
	}
	
	NO_DISCARD FORCEINLINE T& GetValue()
	{
		return *static_cast<T*>(GetComponentValue());
	}
	
	NO_DISCARD FORCEINLINE T* TryGetValue()
	{
		return static_cast<T*>(TryGetComponentValue());
	}
	
}; // struct TFlecsComponentRef