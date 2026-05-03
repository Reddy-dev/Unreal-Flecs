// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsQueryExpression.h"
#include "Queries/FlecsQueryScriptExpr.h"

#include "FlecsQueryScriptExpression.generated.h"

USTRUCT(BlueprintType, DisplayName = "Script Expression")
struct UNREALFLECS_API FFlecsQueryScriptExpression : public FFlecsQueryExpression
{
	GENERATED_BODY()

public:
	FFlecsQueryScriptExpression();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ShowOnlyInnerProperties))
	FFlecsQueryScriptExpr ScriptExpr;

	virtual void Apply(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld, FFlecsQueryBuilderView& InQueryBuilder) const override;
	
}; // struct FFlecsQueryScriptExpression
