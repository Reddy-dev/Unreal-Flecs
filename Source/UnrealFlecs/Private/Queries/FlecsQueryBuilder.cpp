// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Queries/FlecsQueryBuilder.h"

#include "Worlds/FlecsWorld.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsQueryBuilder)

FFlecsQueryBuilder::FFlecsQueryBuilder(const UFlecsWorld* InWorld, const FString& InName)
	: Name(InName)
{
	World = InWorld;
}

FFlecsQuery FFlecsQueryBuilder::Build() const
{
	flecs::query_builder<> Builder = flecs::query_builder<>(World->World, StringCast<char>(*Name).Get());
	Definition.Apply(World.Get(), Builder);
	return FFlecsQuery(Builder.build());
}
