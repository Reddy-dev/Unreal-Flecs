// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Entities/FlecsEntityHandle.h"

#include "FlecsTimerHandle.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsTimerHandle final : public FFlecsEntityHandle
{
	GENERATED_BODY()
	
public:
	using FFlecsEntityHandle::FFlecsEntityHandle;
	
	FORCEINLINE FFlecsTimerHandle(const flecs::timer& InTimer)
		: FFlecsEntityHandle(InTimer.world(), InTimer.id())
	{
	}
	
	FORCEINLINE const FFlecsTimerHandle& SetInterval(const float InInterval) const
	{
		GetTimer().interval(InInterval);
		
		return *this;
	}
	
	NO_DISCARD FORCEINLINE double GetInterval() const
	{
		return GetTimer().interval();
	}
	
	FORCEINLINE const FFlecsTimerHandle& SetTimeout(const float InTimeout) const
	{
		GetTimer().timeout(InTimeout);
		
		return *this;
	}
	
	NO_DISCARD FORCEINLINE double GetTimeout() const
	{
		return GetTimer().timeout();
	}
	
	FORCEINLINE const FFlecsTimerHandle& SetRate(const int32 InRate, const FFlecsId InTickSource = FFlecsId()) const
	{
		GetTimer().rate(InRate, InTickSource);
		
		return *this;
	}
	
	FORCEINLINE void Start() const
	{
		GetTimer().start();
	}
	
	FORCEINLINE void Stop() const
	{
		GetTimer().stop();
	}

	NO_DISCARD FORCEINLINE flecs::timer GetTimer() const
	{
		return flecs::timer(GetNativeFlecsWorld(), GetEntity());
	}
	
	FORCEINLINE operator flecs::timer() const
	{
		return GetTimer();
	}
	
}; // struct FFlecsTimerHandle
