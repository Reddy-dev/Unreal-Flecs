// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "General/FlecsObjectRegistrationInterface.h"
#include "Worlds/FlecsWorldInterfaceObject.h"

#include "FlecsRegisteredObjectDependencyTestTypes.generated.h"

UCLASS(Abstract)
class UNREALFLECSTESTS_API UFlecsRegisteredObjectDependencyTestBase : public UObject,
	public IFlecsObjectRegistrationInterface
{
	GENERATED_BODY()

public:
	virtual void RegisterObject(const TSolidNotNull<UFlecsWorldInterfaceObject*> InFlecsWorld) override
	{
		++RegisterObjectCallCount;
	}

	virtual bool ShouldAutoRegisterFromCDO() const override
	{
		return false;
	}

	virtual EUnrealFlecsRegistrationScopeType GetRegistrationScopeType() const override
	{
		return EUnrealFlecsRegistrationScopeType::None;
	}

	NO_DISCARD int32 GetRegisterObjectCallCount() const
	{
		return RegisterObjectCallCount;
	}

private:
	int32 RegisterObjectCallCount = 0;

}; // class UFlecsRegisteredObjectDependencyTestBase

UCLASS()
class UNREALFLECSTESTS_API UFlecsRegisteredObjectDependencyTestObject final
	: public UFlecsRegisteredObjectDependencyTestBase
{
	GENERATED_BODY()

}; // class UFlecsRegisteredObjectDependencyTestObject

UCLASS()
class UNREALFLECSTESTS_API UFlecsRegisteredObjectSecondDependencyTestObject final
	: public UFlecsRegisteredObjectDependencyTestBase
{
	GENERATED_BODY()

}; // class UFlecsRegisteredObjectSecondDependencyTestObject

UCLASS()
class UNREALFLECSTESTS_API UFlecsRegisteredObjectDependentTestObject final
	: public UFlecsRegisteredObjectDependencyTestBase
{
	GENERATED_BODY()

public:
	virtual void RegisterObject(const TSolidNotNull<UFlecsWorldInterfaceObject*> InFlecsWorld) override
	{
		UFlecsRegisteredObjectDependencyTestBase::RegisterObject(InFlecsWorld);
		bDependencyWasRegistered = InFlecsWorld->IsFlecsObjectRegistered<UFlecsRegisteredObjectDependencyTestObject>();
	}

	virtual TArray<TSubclassOf<UObject>> GetDependentRegistrationClasses() const override
	{
		return { UFlecsRegisteredObjectDependencyTestObject::StaticClass() };
	}

	NO_DISCARD bool WasDependencyRegistered() const
	{
		return bDependencyWasRegistered;
	}

private:
	bool bDependencyWasRegistered = false;

}; // class UFlecsRegisteredObjectDependentTestObject

UCLASS()
class UNREALFLECSTESTS_API UFlecsRegisteredObjectTransitiveDependentTestObject final
	: public UFlecsRegisteredObjectDependencyTestBase
{
	GENERATED_BODY()

public:
	virtual void RegisterObject(const TSolidNotNull<UFlecsWorldInterfaceObject*> InFlecsWorld) override
	{
		UFlecsRegisteredObjectDependencyTestBase::RegisterObject(InFlecsWorld);
		bDependencyWasRegistered = InFlecsWorld->IsFlecsObjectRegistered<UFlecsRegisteredObjectDependentTestObject>();
	}

	virtual TArray<TSubclassOf<UObject>> GetDependentRegistrationClasses() const override
	{
		return { UFlecsRegisteredObjectDependentTestObject::StaticClass() };
	}

	NO_DISCARD bool WasDependencyRegistered() const
	{
		return bDependencyWasRegistered;
	}

private:
	bool bDependencyWasRegistered = false;

}; // class UFlecsRegisteredObjectTransitiveDependentTestObject

UCLASS()
class UNREALFLECSTESTS_API UFlecsRegisteredObjectMultipleDependencyTestObject final
	: public UFlecsRegisteredObjectDependencyTestBase
{
	GENERATED_BODY()

public:
	virtual TArray<TSubclassOf<UObject>> GetDependentRegistrationClasses() const override
	{
		return
		{
			UFlecsRegisteredObjectDependencyTestObject::StaticClass(),
			UFlecsRegisteredObjectSecondDependencyTestObject::StaticClass()
		};
	}

}; // class UFlecsRegisteredObjectMultipleDependencyTestObject

UCLASS()
class UNREALFLECSTESTS_API UFlecsRegisteredObjectCycleATestObject final
	: public UFlecsRegisteredObjectDependencyTestBase
{
	GENERATED_BODY()

public:
	virtual TArray<TSubclassOf<UObject>> GetDependentRegistrationClasses() const override;

}; // class UFlecsRegisteredObjectCycleATestObject

UCLASS()
class UNREALFLECSTESTS_API UFlecsRegisteredObjectCycleBTestObject final
	: public UFlecsRegisteredObjectDependencyTestBase
{
	GENERATED_BODY()

public:
	virtual TArray<TSubclassOf<UObject>> GetDependentRegistrationClasses() const override;

}; // class UFlecsRegisteredObjectCycleBTestObject
