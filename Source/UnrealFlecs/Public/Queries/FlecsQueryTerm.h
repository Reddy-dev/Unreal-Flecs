// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "flecs.h"

#include "CoreMinimal.h"

#include "StructUtils/InstancedStruct.h"

#include "Types/SolidNotNull.h"

#include "FlecsQueryTerm.generated.h"

struct FFlecsQueryGeneratorInputType;

class UFlecsWorld;

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsQueryTerm
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
	TInstancedStruct<FFlecsQueryGeneratorInputType> InputType;

	void ApplyToQueryBuilder(const TSolidNotNull<const UFlecsWorld*> InWorld, flecs::query_builder<>& InQueryBuilder) const;
	
}; // struct FFlecsQueryTerm