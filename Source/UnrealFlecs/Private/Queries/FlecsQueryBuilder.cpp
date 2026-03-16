// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Queries/FlecsQueryBuilder.h"

#include "Queries/FlecsQueryBuilderView.h"
#include "Worlds/FlecsWorld.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsQueryBuilder)

FFlecsQueryBuilder::FFlecsQueryBuilder(const UFlecsWorld* InWorld, const FString& InQueryName)
	: QueryName(InQueryName)
{
	World = InWorld;
}

FFlecsQueryBuilder::FFlecsQueryBuilder(const UFlecsWorld* InWorld, const FFlecsEntityHandle& InQueryEntity)
{
	World = InWorld;
	OptionalQueryEntity = InQueryEntity;
}

FFlecsQuery FFlecsQueryBuilder::Build() const
{
	if (OptionalQueryEntity.IsSet())
	{
		const FFlecsEntityHandle QueryEntity = OptionalQueryEntity.GetValue();
		solid_checkf(QueryEntity.IsValid(), TEXT("Invalid query entity provided to FlecsQueryBuilder"));
		
		flecs::query_builder<> Builder = flecs::query_builder<>(World->World, QueryEntity);
		FFlecsQueryBuilderView BuilderView = MakeQueryBuilderView_Internal(Builder);
		Definition.Apply(World.Get(), BuilderView);
		return FFlecsQuery(Builder.build());
	}
	else
	{
		flecs::query_builder<> Builder = flecs::query_builder<>(World->World, StringCast<char>(*QueryName).Get());
		FFlecsQueryBuilderView BuilderView = MakeQueryBuilderView_Internal(Builder);
		Definition.Apply(World.Get(), BuilderView);
		return FFlecsQuery(Builder.build());
	}
}
