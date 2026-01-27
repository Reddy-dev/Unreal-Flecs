// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsQueryBuilderBase.h"
#include "FlecsQuery.h"

#include "FlecsQueryBuilder.generated.h"

USTRUCT()
struct UNREALFLECS_API FFlecsQueryBuilder 
#if CPP
	: public TFlecsQueryBuilderBase<FFlecsQueryBuilder>
#endif // CPP
{
	GENERATED_BODY()
	
public:
	FORCEINLINE FFlecsQueryBuilder() = default;
	
	explicit FFlecsQueryBuilder(const TSolidNotNull<const UFlecsWorld*> InWorld);
	
	NO_DISCARD FORCEINLINE FFlecsQueryDefinition& GetQueryDefinition_Impl() const
	{
		return const_cast<FFlecsQueryBuilder*>(this)->Definition;
	}
	
	FORCEINLINE FFlecsQuery Build() const
	{
		return FFlecsQuery(World.Get(), Definition);
	}
	
	UPROPERTY()
	TWeakObjectPtr<const UFlecsWorld> World;
	
	UPROPERTY()
	FFlecsQueryDefinition Definition;
	
}; // struct FFlecsQueryBuilder