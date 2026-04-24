// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "flecs.h"

#include "CoreMinimal.h"

#include "Types/SolidNotNull.h"

#include "Generator/FlecsQueryGeneratorInput.h"

#include "FlecsQueryTerm.generated.h"

struct FFlecsQueryBuilderView;
struct FFlecsQueryGeneratorInputType;

class UFlecsWorldInterfaceObject;

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsQueryTerm
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
	FFlecsQueryGeneratorInput Input;

	void ApplyToQueryBuilder(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld, FFlecsQueryBuilderView& InQueryBuilder) const;
	
}; // struct FFlecsQueryTerm