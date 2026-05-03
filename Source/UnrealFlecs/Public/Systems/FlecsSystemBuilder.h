// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsSystemBuilderBase.h"
#include "FlecsSystemHandle.h"
#include "Queries/FlecsQueryBuilder.h"

template <typename ...TComponents>
struct TFlecsSystemBuilder : public TFlecsSystemBuilderBase<TFlecsSystemBuilder<TComponents...>, FFlecsSystemHandle, TComponents...>
{
	friend struct TFlecsSystemBuilderBase<TFlecsSystemBuilder, FFlecsSystemHandle, TComponents...>;
	
public:
	FORCEINLINE FFlecsSystemDefinition& GetSystemDefinition_Impl() const
	{
		return const_cast<FFlecsSystemDefinition&>(SystemDefinition);
	}
	
	FORCEINLINE TFlecsSystemBuilder(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld, const FString& InOptionalName, 
		const FFlecsSystemDefinition& InObserverDefinition = FFlecsSystemDefinition())
									: SystemDefinition(InObserverDefinition)
									, FlecsWorld(InWorld)
									, OptionalName(InOptionalName)
	{
		UE::Flecs::Queries::TAddInputTypes<TFlecsSystemBuilder, TComponents...>::Apply(*this);
	}
	
	FFlecsSystemDefinition SystemDefinition;
	
	TWeakObjectPtr<const UFlecsWorldInterfaceObject> FlecsWorld;
	
	FString OptionalName;
	
protected:
	
	FORCEINLINE FFlecsSystemHandle CreateSystem() const
	{
		solid_checkf(FlecsWorld.IsValid(), TEXT("World is not valid."));
		
		return FFlecsSystemHandle(FlecsWorld.Get(), SystemDefinition, OptionalName);
	}
	
	FORCEINLINE FFlecsSystemHandle CreateRunSystem() const
	{
		return CreateSystem();
	}
	
	FORCEINLINE FFlecsSystemHandle CreateRunEachSystem() const
	{
		return CreateSystem();
	}
	
	FORCEINLINE FFlecsSystemHandle CreateEachSystem() const
	{
		return CreateSystem();
	}
	
}; // struct TFlecsSystemBuilder

