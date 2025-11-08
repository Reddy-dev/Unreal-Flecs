// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FlecsDependenciesComponent.h"

#include "UObject/Object.h"

#include "FlecsModuleInterface.h"

#include "FlecsModuleObject.generated.h"

class UFlecsWorld;

UCLASS(Abstract, Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced, ClassGroup = (Flecs))
class UNREALFLECS_API UFlecsModuleObject : public UObject, public IFlecsModuleInterface
{
	GENERATED_BODY()

public:
	UFlecsModuleObject();
	UFlecsModuleObject(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditDefaultsOnly, Category = "Flecs Module",
		meta = (NoElementDuplicate, MustImplement = "/Script/UnrealFlecs.FlecsModuleInterface"))
	TArray<TSubclassOf<UFlecsModuleInterface>> HardModuleDependencies;
	
	template <Solid::TStaticClassConcept T>
	NO_DISCARD bool HasModuleDependency() const
	{
		return HasModuleDependency(T::StaticClass());
	}

	UFUNCTION()
	bool HasHardDependency(TSubclassOf<UFlecsModuleInterface> ModuleClass) const;

	template <Solid::TStaticClassConcept T>
	void AddHardDependency()
	{
		AddDependency(T::StaticClass());
	}
	
	void AddHardDependency(TSubclassOf<UFlecsModuleInterface> ModuleClass);
	
	void RegisterSoftDependency(const TSubclassOf<UFlecsModuleInterface> ModuleClass,
		const FFlecsDependencyFunctionDefinition::FDependencyFunctionType& DependencyFunction);

	template <Solid::TStaticClassConcept T>
	FORCEINLINE void RegisterSoftDependency(const FFlecsDependencyFunctionDefinition::FDependencyFunctionType& DependencyFunction)
	{
		RegisterSoftDependency(T::StaticClass(), DependencyFunction);
	}

	virtual TArray<TSubclassOf<UFlecsModuleInterface>> GetHardDependentModuleClasses() const override;
	
}; // class UFlecsModuleObject
