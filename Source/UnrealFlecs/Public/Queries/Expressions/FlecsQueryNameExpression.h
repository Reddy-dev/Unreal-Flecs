// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsQueryExpression.h"

#include "FlecsQueryNameExpression.generated.h"

USTRUCT(BlueprintType, meta = (DisplayName = "Name Expression"))
struct UNREALFLECS_API FFlecsQueryNameExpression : public FFlecsQueryExpression
{
	GENERATED_BODY()

public:
	FFlecsQueryNameExpression();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	virtual void Apply(const TSolidNotNull<const UFlecsWorld*> InWorld, FFlecsQueryBuilderView& InQueryBuilder) const override;
	
}; // struct FFlecsQueryNameExpression