// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "flecs.h"

#include "CoreMinimal.h"

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
	
	FFlecsSystemHandle(const TSolidNotNull<const UFlecsWorld*> InWorld, 
		const FFlecsSystemDefinition& InSystemBuilder, const FString& InSystemName);
	
	FORCEINLINE FFlecsSystemHandle(const flecs::system& InSystem)
		: FFlecsEntityHandle(InSystem.world(), InSystem.id())
	{
	}
	
	FORCEINLINE void SetContext(void* InContext) const
	{
		GetSystem().ctx(InContext);
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
	
	flecs::system_runner_fluent RunWorker(const int32 StageCurrent, const int32 StageCount, const double InDeltaTime = 0.0, void* InParams = nullptr) const
	{
		return GetSystem().run_worker(StageCurrent, StageCount, InDeltaTime, InParams);
	}

	NO_DISCARD FORCEINLINE flecs::system GetSystem() const
	{
		return flecs::system(GetNativeFlecsWorld(), GetFlecsId());
	}
	
	FORCEINLINE FFlecsSystemHandle SetGroup(const uint64 InGroupId) const
	{
		GetSystem().set_group(InGroupId);
		return *this;
	}
	
	template <typename T>
	FORCEINLINE FFlecsSystemHandle SetGroup() const
	{
		GetSystem().set_group<T>();
		return *this;
	}
	
}; // struct FFlecsSystemHandle


