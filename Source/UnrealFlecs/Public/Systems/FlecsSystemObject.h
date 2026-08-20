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
	
	virtual void RegisterObject(const TSolidNotNull<UFlecsWorldInterfaceObject*> InFlecsWorld) override;
	virtual void UnregisterObject(const TSolidNotNull<UFlecsWorldInterfaceObject*> InFlecsWorld) override;
	virtual void FlecsWorldBeginPlay(const TSolidNotNull<UFlecsWorldInterfaceObject*> InFlecsWorld) override;
	
	virtual NO_DISCARD EFlecsObjectRegistrationNetworkFlags GetObjectRegistrationNetworkFlags() const override
	{
		return NetworkRegistrationFlags;
	}
	
	void SetContext(void* InContext) const;
	
	void RunSystem(const double InDeltaTime = 0.0, void* InParams = nullptr) const;
	
	NO_DISCARD FORCEINLINE FFlecsSystemDefinition& GetSystemDefinition()
	{
		return SystemDefinition;
	}
	
	NO_DISCARD FORCEINLINE const FFlecsSystemDefinition& GetSystemDefinition() const
	{
		return SystemDefinition;
	}

	#if WITH_EDITORONLY_DATA
	
	virtual NO_DISCARD bool ShouldShowInSettings() const override { return true; }
	
#endif // WITH_EDITORONLY_DATA
	
protected:
	UPROPERTY(Transient)
	FFlecsSystemHandle SystemHandle;
	
	UPROPERTY(EditAnywhere, Config, Category = "Flecs", meta = (AllowPrivateAccess = "true"))
	EFlecsObjectRegistrationNetworkFlags NetworkRegistrationFlags = EFlecsObjectRegistrationNetworkFlags::All;
	
	// Set in BuildObserver
	UPROPERTY(EditAnywhere, Config, Category = "Flecs", meta = (AllowPrivateAccess = "true"))
	FFlecsSystemDefinition SystemDefinition;
	
private:
	void InitializeSystem(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld);
	
}; // class UFlecsSystemObject
