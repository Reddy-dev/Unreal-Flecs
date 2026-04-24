// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FlecsSystemBuilder.h"

#include "UObject/Object.h"

#include "FlecsSystemHandleInterface.h"
#include "General/FlecsObjectRegistrationInterface.h"
#include "Queries/FlecsIteratorObjectInterface.h"
#include "FlecsSystemDefinition.h"

#include "FlecsSystemObject.generated.h"

UCLASS(Abstract, BlueprintType, NotBlueprintable)
class UNREALFLECS_API UFlecsSystemObject : public UObject, public IFlecsSystemHandleInterface
	, public IFlecsIteratorObjectInterface, public IFlecsObjectRegistrationInterface
{
	GENERATED_BODY()

public:
	UFlecsSystemObject();
	UFlecsSystemObject(const FObjectInitializer& ObjectInitializer);
	
	NO_DISCARD FORCEINLINE virtual FFlecsSystemHandle GetSystemHandle() const override final
	{
		return SystemHandle;
	}
	
	virtual void BuildSystem(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld, TFlecsSystemBuilder<>& InBuilder) const;
	
	UFUNCTION(BlueprintCallable, Category = "Flecs|Observer")
	UFlecsWorld* GetFlecsWorld() const;
	
	virtual void RegisterObject(const TSolidNotNull<UFlecsWorldInterfaceObject*> InFlecsWorld) override final;
	virtual void UnregisterObject(const TSolidNotNull<UFlecsWorldInterfaceObject*> InFlecsWorld) override;
	virtual void FlecsWorldBeginPlay(const TSolidNotNull<UFlecsWorldInterfaceObject*> InFlecsWorld) override;
	
	void SetContext(void* InContext) const;
	
	void RunSystem(const double InDeltaTime = 0.0, void* InParams = nullptr) const;
	
protected:
	UPROPERTY(Transient)
	FFlecsSystemHandle SystemHandle;
	
	UPROPERTY(EditAnywhere, Category = "Flecs", meta = (AllowPrivateAccess = "true"))
	FFlecsSystemDefinition SystemDefinition;
	
private:
	void InitializeSystem(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld);
	
}; // class UFlecsSystemObject
