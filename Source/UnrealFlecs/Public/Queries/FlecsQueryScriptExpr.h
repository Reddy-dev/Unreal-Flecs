// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SolidMacros/Macros.h"

#include "FlecsQueryScriptExpr.generated.h"

/**
 * @TODO: We need this type to have a custom PropertyEditor for the FlecsQueryScriptExpr type.
 */
USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsQueryScriptExpr
{
	GENERATED_BODY()
	
	static NO_DISCARD FFlecsQueryScriptExpr FromString(const FString& InString)
	{
		return FFlecsQueryScriptExpr(InString);
	}

public:
	UPROPERTY(EditAnywhere)
	FString Expr;
	
}; // struct FFlecsQueryScriptExpr
