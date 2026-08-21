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

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsObserverDefinitionOverrides
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	bool bOverrideObserverEvents = false;
	
	/**
	 * if bOverrideObserverEvents is true, this will be used instead of the Events in the ObserverDefinition, 
	 * otherwise the Events in the ObserverDefinition will be used and these will only be appended
	 **/
	UPROPERTY(EditAnywhere)
	TArray<FFlecsObserverEventInput> EventsOverride;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TOptional<bool> YieldExistingOverride;
	
	/**
	 * if bOverrideObserverFlags is true, this will be used instead of the Flags in the ObserverDefinition, 
	 * otherwise the Flags in the ObserverDefinition will be used
	 **/
	UPROPERTY(EditAnywhere)
	bool bOverrideObserverFlags = false;
	
	UPROPERTY(EditAnywhere, meta = (Bitmask, BitmaskEnum = "/Script/UnrealFlecs.EFlecsObserverFlags"))
	uint32 FlagsOverride = static_cast<uint32>(EFlecsObserverFlags::None);
	
}; // struct FFlecsObserverDefinitionOverrides

UCLASS(Abstract, BlueprintType, NotBlueprintable, Config = Flecs, DefaultConfig)
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
	
	virtual void BuildObserver(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld, TFlecsObserverBuilder<>& InOutBuilder) const;
	
	UFUNCTION(BlueprintCallable, Category = "Flecs|Observer")
	UFlecsWorld* GetFlecsWorld() const;
	
	virtual void RegisterObject(const TSolidNotNull<UFlecsWorldInterfaceObject*> InFlecsWorld) override;
	virtual void UnregisterObject(const TSolidNotNull<UFlecsWorldInterfaceObject*> InFlecsWorld) override;
	virtual void FlecsWorldBeginPlay(const TSolidNotNull<UFlecsWorldInterfaceObject*> InFlecsWorld) override;
	
	virtual NO_DISCARD uint8 GetObjectRegistrationNetworkFlags() const override
	{
		return NetworkRegistrationFlags;
	}
	
#if WITH_EDITORONLY_DATA
	
	virtual NO_DISCARD bool ShouldShowInSettings() const override { return true; }
	
#endif // WITH_EDITORONLY_DATA

protected:
	UPROPERTY(Transient)
	FFlecsObserverHandle ObserverHandle;
	
	UPROPERTY(EditAnywhere, Config, Category = "Flecs", meta = (AllowPrivateAccess = "true", 
		Bitmask, BitmaskEnum = "/Script/UnrealFlecs.EFlecsObjectRegistrationNetworkFlags"))
	uint8 NetworkRegistrationFlags = static_cast<uint8>(EFlecsObjectRegistrationNetworkFlags::All);
	
	UPROPERTY(EditAnywhere, Config, Category = "Flecs", meta = (AllowPrivateAccess = "true"))
	FFlecsObserverDefinitionOverrides ObserverDefinitionOverrides;
	
	UPROPERTY(EditDefaultsOnly, Config, Category = "Flecs", meta = (AllowPrivateAccess = "true"))
	FFlecsObserverDefinition ObserverDefinition;
	
	void ApplyObserverDefinitionOverrides(FFlecsObserverDefinition& InOutDefinition) const;
	
private:
	
	void InitializeObserver(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld);
	
}; // class UFlecsObserverObject
