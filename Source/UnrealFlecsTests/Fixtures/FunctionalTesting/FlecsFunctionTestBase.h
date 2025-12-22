// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FunctionalTest.h"

#include "SolidMacros/Macros.h"
#include "Types/SolidNotNull.h"

#include "FlecsFunctionTestBase.generated.h"

class UFlecsWorld;

UCLASS(Abstract, Blueprintable)
class UNREALFLECSTESTS_API AFlecsFunctionTestBase : public AFunctionalTest
{
	GENERATED_BODY()

public:
	AFlecsFunctionTestBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void PrepareTest() override;

	UFUNCTION(BlueprintCallable, Category = "Flecs | Testing")
	UFlecsWorld* GetOwningFlecsWorld() const;

	NO_DISCARD TSolidNotNull<UFlecsWorld*> GetOwningFlecsWorldChecked() const;
	
private:
	UPROPERTY(Transient)
	TWeakObjectPtr<UFlecsWorld> OwningFlecsWorld;
	
}; // class AFlecsFunctionTestBase
