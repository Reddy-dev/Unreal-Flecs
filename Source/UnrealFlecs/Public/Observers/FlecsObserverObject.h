// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "UObject/Object.h"

#include "SolidMacros/Macros.h"

#include "Queries/FlecsIteratorObjectInterface.h"

#include "FlecsObserverDefinition.h"
#include "FlecsObserverHandleInterface.h"
#include "FlecsObserverBuilder.h"
#include "General/FlecsObjectRegistrationInterface.h"

#include "FlecsObserverObject.generated.h"

UCLASS(Abstract, BlueprintType, NotBlueprintable)
class UNREALFLECS_API UFlecsObserverObject : public UObject, 
	public IFlecsObserverHandleInterface, public IFlecsIteratorObjectInterface, public IFlecsObjectRegistrationInterface
{
	GENERATED_BODY()

public:
	UFlecsObserverObject();
	UFlecsObserverObject(const FObjectInitializer& ObjectInitializer);
	
	NO_DISCARD FORCEINLINE virtual FFlecsObserverHandle GetObserverHandle() const override final
	{
		return ObserverHandle;
	}
	
	virtual void BuildObserver(const TSolidNotNull<const UFlecsWorld*> InWorld, TFlecsObserverBuilder<>& InOutBuilder) const;
	
	UFUNCTION(BlueprintCallable, Category = "Flecs|Observer")
	UFlecsWorld* GetFlecsWorld() const;
	
	virtual void RegisterObject(const TSolidNotNull<UFlecsWorld*> InFlecsWorld) override final;
	virtual void UnregisterObject(const TSolidNotNull<UFlecsWorld*> InFlecsWorld) override;
	virtual void FlecsWorldBeginPlay(const TSolidNotNull<UFlecsWorld*> InFlecsWorld) override;

protected:
	UPROPERTY(Transient)
	FFlecsObserverHandle ObserverHandle;
	
	UPROPERTY(EditAnywhere, Category = "Flecs", meta = (AllowPrivateAccess = "true"))
	FFlecsObserverDefinition ObserverDefinition;
	
private:
	
	void InitializeObserver(const TSolidNotNull<UFlecsWorld*> InWorld);
	
}; // class UFlecsObserverObject
