// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "flecs.h"


#include "Entities/FlecsEntityHandle.h"
#include "Queries/FlecsQuery.h"

#include "FlecsSystemHandle.generated.h"

struct FFlecsSystemDefinition;

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsSystemHandle : public FFlecsEntityHandle
{
	GENERATED_BODY()
	
public:
	using FFlecsEntityHandle::FFlecsEntityHandle;
	
	FFlecsSystemHandle(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld, 
		const FFlecsSystemDefinition& InSystemBuilder, const FString& InSystemName);
	
	FFlecsSystemHandle(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld,
		const FFlecsSystemDefinition& InSystemBuilder, const FFlecsId InExistingEntity);

	FORCEINLINE FFlecsSystemHandle(const flecs::system& InSystem)
		: FFlecsEntityHandle(InSystem.world(), InSystem.id())
	{
	}
	
	template <typename TSelf>
	FORCEINLINE const TSelf& SetContext(this const TSelf& InSelf, void* InContext)
	{
		InSelf.GetSystem().ctx(InContext);
		return InSelf;
	}
	
	NO_DISCARD FORCEINLINE void* GetContext() const
	{
		return GetSystem().ctx();
	}
	
	FORCEINLINE NO_DISCARD FFlecsQuery GetQuery() const
	{
		return GetSystem().query();
	}
	
	flecs::system_runner_fluent Run(const double InDeltaTime = 0.0, void* InParams = nullptr) const
	{
		return GetSystem().run(InDeltaTime, InParams);
	}
	
	flecs::system_runner_fluent RunWorker(const int32 StageCurrent, const int32 StageCount, 
		const double InDeltaTime = 0.0, void* InParams = nullptr) const
	{
		return GetSystem().run_worker(StageCurrent, StageCount, InDeltaTime, InParams);
	}

	NO_DISCARD FORCEINLINE flecs::system GetSystem() const
	{
		return flecs::system(GetNativeFlecsWorld(), GetFlecsId());
	}
	
	template <typename TSelf>
	FORCEINLINE const TSelf& SetGroup(this const TSelf& InSelf, const uint64 InGroupId)
	{
		InSelf.GetSystem().set_group(InGroupId);
		return InSelf;
	}
	
	template <typename T, typename TSelf>
	FORCEINLINE const TSelf& SetGroup(this const TSelf& InSelf)
	{
		InSelf.GetSystem().template set_group<T>();
		return InSelf;
	}
	
	template <typename TSelf>
	FORCEINLINE const TSelf& SetInterval(this const TSelf& InSelf, const double InInterval)
	{
		InSelf.GetSystem().interval(InInterval);
		return InSelf;
	}
	
	NO_DISCARD FORCEINLINE double GetInterval() const
	{
		return GetSystem().interval();
	}
	
	template <typename TSelf>
	FORCEINLINE const TSelf& SetTimeout(this const TSelf& InSelf, const double InTimeout)
	{
		InSelf.GetSystem().timeout(InTimeout);
		return InSelf;
	}
	
	NO_DISCARD FORCEINLINE double GetTimeout() const
	{
		return GetSystem().timeout();
	}
	
	template <typename TSelf>
	FORCEINLINE const TSelf& SetRate(this const TSelf& InSelf, const int32 InRate)
	{
		InSelf.GetSystem().rate(InRate);
		return InSelf;
	}
	
	template <typename TSelf>
	FORCEINLINE const TSelf& StartTimer(this const TSelf& InSelf)
	{
		InSelf.GetSystem().start();
		return InSelf;
	}
	
	template <typename TSelf>
	FORCEINLINE const TSelf& StopTimer(this const TSelf& InSelf)
	{
		InSelf.GetSystem().stop();
		return InSelf;
	}
	
	template <typename TSelf>
	FORCEINLINE const TSelf& SetTickSource(this const TSelf& InSelf, const FFlecsId InTickSource)
	{
		InSelf.GetSystem().set_tick_source(InTickSource);
		return InSelf;
	}
	
	template <typename T, typename TSelf>
	FORCEINLINE const TSelf& SetTickSource(this const TSelf& InSelf)
	{
		InSelf.GetSystem().template set_tick_source<T>();
		return InSelf;
	}
	
}; // struct FFlecsSystemHandle
