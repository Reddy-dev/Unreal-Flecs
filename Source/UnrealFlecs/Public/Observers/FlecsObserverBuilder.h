// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsObserverBuilderBase.h"
#include "FlecsObserverHandle.h"
#include "Queries/FlecsQueryBuilder.h"

class UFlecsWorld;

template <typename ...TComponents>
struct TFlecsObserverBuilder : public TFlecsObserverBuilderBase<TFlecsObserverBuilder<TComponents...>, FFlecsObserverHandle, TComponents...>
{
	friend struct TFlecsObserverBuilderBase<TFlecsObserverBuilder<TComponents...>, FFlecsObserverHandle, TComponents...>;
	
public:
	FORCEINLINE FFlecsObserverDefinition& GetObserverDefinition_Impl() const
	{
		return const_cast<FFlecsObserverDefinition&>(ObserverDefinition);
	}
	
	FORCEINLINE TFlecsObserverBuilder(const TSolidNotNull<const UFlecsWorld*> InWorld, const FString& InOptionalName, 
		const FFlecsObserverDefinition& InObserverDefinition = FFlecsObserverDefinition())
									: ObserverDefinition(InObserverDefinition)
									, World(InWorld)
									, OptionalName(InOptionalName)
	{
		Unreal::Flecs::Queries::TAddInputTypes<TFlecsObserverBuilder, TComponents...>::Apply(*this);
	}

	FFlecsObserverDefinition ObserverDefinition;
	
	TWeakObjectPtr<const UFlecsWorld> World;
	
	FString OptionalName;
	
protected:
	
	FORCEINLINE FFlecsObserverHandle CreateObserver() const
	{
		solid_checkf(World.IsValid(), TEXT("World is not valid."));
		
		return FFlecsObserverHandle(World.Get(), ObserverDefinition, OptionalName);
	}

	FORCEINLINE FFlecsObserverHandle CreateRunObserver() const
	{
		return CreateObserver();
	}
	
	FORCEINLINE FFlecsObserverHandle CreateRunEachObserver() const
	{
		return CreateObserver();
	}
	
	FORCEINLINE FFlecsObserverHandle CreateEachObserver() const
	{
		return CreateObserver();
	}
	
}; // struct TFlecsObserverBuilder
