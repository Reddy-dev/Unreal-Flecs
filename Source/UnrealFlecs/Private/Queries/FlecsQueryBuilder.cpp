// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Queries/FlecsQueryBuilder.h"

#include "Queries/FlecsQueryBuilderView.h"
#include "Worlds/FlecsWorldInterfaceObject.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsQueryBuilder)

FFlecsQueryBuilder::FFlecsQueryBuilder(const UFlecsWorldInterfaceObject* InWorld, const FString& InQueryName)
	: QueryName(InQueryName)
{
	FlecsWorld = InWorld;
}

FFlecsQueryBuilder::FFlecsQueryBuilder(const UFlecsWorldInterfaceObject* InWorld, const FFlecsEntityHandle& InQueryEntity)
{
	FlecsWorld = InWorld;
	OptionalQueryEntity = InQueryEntity;
}

FFlecsQuery FFlecsQueryBuilder::Build() const
{
	if (OptionalQueryEntity.IsSet())
	{
		const FFlecsEntityHandle QueryEntity = OptionalQueryEntity.GetValue();
		solid_checkf(QueryEntity.IsValid(), TEXT("Invalid query entity provided to FlecsQueryBuilder"));
		
		flecs::query_builder<> Builder = flecs::query_builder<>(FlecsWorld->GetNativeFlecsWorld(), QueryEntity);
		FFlecsQueryBuilderView BuilderView = MakeQueryBuilderView_Internal(Builder);
		Definition.Apply(FlecsWorld.Get(), BuilderView);
		return FFlecsQuery(Builder.build());
	}
	else
	{
		flecs::query_builder<> Builder = flecs::query_builder<>(FlecsWorld->GetNativeFlecsWorld(), StringCast<char>(*QueryName).Get());
		FFlecsQueryBuilderView BuilderView = MakeQueryBuilderView_Internal(Builder);
		Definition.Apply(FlecsWorld.Get(), BuilderView);
		return FFlecsQuery(Builder.build());
	}
}
