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
	
	FORCEINLINE NO_DISCARD FFlecsQuery GetQuery() const
	{
		return GetSystem().query();
	}
	
	flecs::system_runner_fluent Run(const double InDeltaTime = 0.0, void* InParams = nullptr) const
	{
		return GetSystem().run(InDeltaTime, InParams);
	}

	NO_DISCARD FORCEINLINE flecs::system GetSystem() const
	{
		return flecs::system(GetNativeFlecsWorld(), GetFlecsId());
	}
	
}; // struct FFlecsSystemHandle


