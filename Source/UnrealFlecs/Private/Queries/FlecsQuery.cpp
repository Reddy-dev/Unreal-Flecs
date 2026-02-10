// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Queries/FlecsQuery.h"
#include "Components/FlecsWorldPtrComponent.h"
#include "Queries/FlecsQueryBuilder.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsQuery)

/*FFlecsQuery::FFlecsQuery(const TSolidNotNull<const UFlecsWorld*> InFlecsWorld,
                         const FFlecsQueryDefinition& InDefinition,
                         const FString& InName)
{
	flecs::query_builder<> QueryBuilder = InFlecsWorld->World.query_builder<>(StringCast<char>(*InName).Get());
	InDefinition.Apply(InFlecsWorld, QueryBuilder);
	Query = QueryBuilder.build();
}

FFlecsQuery::FFlecsQuery(const FFlecsQueryBuilder& InQueryBuilder)
	: FFlecsQuery(InQueryBuilder.World.Get(), InQueryBuilder.Definition)
{
}*/
