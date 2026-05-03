// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsObserverBuilderBase.h"
#include "FlecsObserverHandle.h"
#include "Queries/FlecsQueryBuilder.h"

class UFlecsWorldInterfaceObject;

template <typename ...TComponents>
struct TFlecsObserverBuilder : public TFlecsObserverBuilderBase<TFlecsObserverBuilder<TComponents...>, FFlecsObserverHandle, TComponents...>
{
	friend struct TFlecsObserverBuilderBase<TFlecsObserverBuilder<TComponents...>, FFlecsObserverHandle, TComponents...>;
	
public:
	FORCEINLINE FFlecsObserverDefinition& GetObserverDefinition_Impl() const
	{
		return const_cast<FFlecsObserverDefinition&>(ObserverDefinition);
	}
	
	FORCEINLINE TFlecsObserverBuilder(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld, const FString& InOptionalName, 
		const FFlecsObserverDefinition& InObserverDefinition = FFlecsObserverDefinition())
									: ObserverDefinition(InObserverDefinition)
									, FlecsWorld(InWorld)
									, OptionalName(InOptionalName)
	{
		UE::Flecs::Queries::TAddInputTypes<TFlecsObserverBuilder, TComponents...>::Apply(*this);
	}

	FFlecsObserverDefinition ObserverDefinition;
	
	TWeakObjectPtr<const UFlecsWorldInterfaceObject> FlecsWorld;
	
	FString OptionalName;
	
protected:
	
	FORCEINLINE FFlecsObserverHandle CreateObserver() const
	{
		solid_checkf(FlecsWorld.IsValid(), TEXT("World is not valid."));
		
		return FFlecsObserverHandle(FlecsWorld.Get(), ObserverDefinition, OptionalName);
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
