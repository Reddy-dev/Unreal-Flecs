// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once


#include "Entities/FlecsEntityHandle.h"
#include "General/FlecsObjectRegistrationInterface.h"
#include "Properties/FlecsComponentProperties.h"
#include "Worlds/FlecsWorldInterfaceObject.h"

#include "FlecsRegistrationScopeTestTypes.generated.h"

namespace UE::Flecs::Tests::RegistrationScope
{
	inline const FName ModuleScopeName = FName(TEXT("RegistrationScopeModule"));
	inline const FName PluginScopeName = FName(TEXT("RegistrationScopePlugin"));
	inline const FName CustomNameScopeName = FName(TEXT("RegistrationScopeCustomName"));
	inline const FName MissingCustomNameScopeName = FName(TEXT("RegistrationScopeMissingCustomName"));

} // namespace UE::Flecs::Tests::RegistrationScope

USTRUCT()
struct FFlecsRegistrationScopeModuleComponent
{
	GENERATED_BODY()
}; // struct FFlecsRegistrationScopeModuleComponent

template <>
struct TFlecsComponentTraits<FFlecsRegistrationScopeModuleComponent>
	: public TFlecsComponentTraitsBase<FFlecsRegistrationScopeModuleComponent>
{
	static constexpr EUnrealFlecsRegistrationScopeType RegistrationScopeType = EUnrealFlecsRegistrationScopeType::Module;

	static FName GetRegistrationScopeName()
	{
		return UE::Flecs::Tests::RegistrationScope::ModuleScopeName;
	}
	
}; // struct TFlecsComponentTraits<FFlecsRegistrationScopeModuleComponent>

USTRUCT()
struct FFlecsRegistrationScopePluginComponent
{
	GENERATED_BODY()
}; // struct FFlecsRegistrationScopePluginComponent

template <>
struct TFlecsComponentTraits<FFlecsRegistrationScopePluginComponent>
	: public TFlecsComponentTraitsBase<FFlecsRegistrationScopePluginComponent>
{
	static constexpr EUnrealFlecsRegistrationScopeType RegistrationScopeType = EUnrealFlecsRegistrationScopeType::Plugin;

	static FName GetRegistrationScopeName()
	{
		return UE::Flecs::Tests::RegistrationScope::PluginScopeName;
	}
	
}; // struct TFlecsComponentTraits<FFlecsRegistrationScopePluginComponent>

UCLASS(Abstract)
class UNREALFLECSTESTS_API UFlecsRegistrationScopeTestObject : public UObject,
	public IFlecsObjectRegistrationInterface
{
	GENERATED_BODY()

public:
	virtual void RegisterObject(const TSolidNotNull<UFlecsWorldInterfaceObject*> InFlecsWorld) override
	{
		RegisteredEntity = InFlecsWorld->CreateEntity(TEXT("RegisteredObject"));
	}

	virtual void FlecsWorldBeginPlay(const TSolidNotNull<UFlecsWorldInterfaceObject*> InFlecsWorld) override
	{
		BeginPlayEntity = InFlecsWorld->CreateEntity(TEXT("BeginPlayObject"));
	}

	NO_DISCARD const FFlecsEntityHandle& GetRegisteredEntity() const
	{
		return RegisteredEntity;
	}

	NO_DISCARD const FFlecsEntityHandle& GetBeginPlayEntity() const
	{
		return BeginPlayEntity;
	}

	void ResetBeginPlayEntity()
	{
		BeginPlayEntity = FFlecsEntityHandle::GetNullHandle();
	}

private:
	FFlecsEntityHandle RegisteredEntity;
	FFlecsEntityHandle BeginPlayEntity;
}; // class UFlecsRegistrationScopeTestObject

UCLASS()
class UNREALFLECSTESTS_API UFlecsExplicitModuleRegistrationScopeTestObject final
	: public UFlecsRegistrationScopeTestObject
{
	GENERATED_BODY()

public:
	virtual EUnrealFlecsRegistrationScopeType GetRegistrationScopeType() const override
	{
		return EUnrealFlecsRegistrationScopeType::Module;
	}

	virtual FName GetScopeName() const override
	{
		return UE::Flecs::Tests::RegistrationScope::ModuleScopeName;
	}
	
	virtual bool ShouldAutoRegisterFromCDO() const override
	{
		return false;
	}
	
}; // class UFlecsExplicitModuleRegistrationScopeTestObject

UCLASS()
class UNREALFLECSTESTS_API UFlecsMissingCustomRegistrationScopeTestObject final
	: public UFlecsRegistrationScopeTestObject
{
	GENERATED_BODY()

public:
	virtual EUnrealFlecsRegistrationScopeType GetRegistrationScopeType() const override
	{
		return EUnrealFlecsRegistrationScopeType::CustomNameIdentifier;
	}

	virtual FName GetScopeName() const override
	{
		return UE::Flecs::Tests::RegistrationScope::MissingCustomNameScopeName;
	}
}; // class UFlecsMissingCustomRegistrationScopeTestObject
