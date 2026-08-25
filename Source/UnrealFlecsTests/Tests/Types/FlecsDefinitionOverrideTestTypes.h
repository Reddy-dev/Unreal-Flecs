// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once


#include "Observers/FlecsObserverObject.h"
#include "Systems/FlecsSystemObject.h"

#include "FlecsDefinitionOverrideTestTypes.generated.h"

UCLASS()
class UNREALFLECSTESTS_API UFlecsSystemDefinitionOverrideTestObject final : public UFlecsSystemObject
{
	GENERATED_BODY()

public:
	void SetDefinitionOverrides(const FFlecsSystemDefinitionOverrides& InOverrides)
	{
		SystemDefinitionOverrides = InOverrides;
	}

	FFlecsSystemDefinitionOverrides& GetDefinitionOverrides()
	{
		return SystemDefinitionOverrides;
	}

	const FFlecsSystemDefinitionOverrides& GetDefinitionOverrides() const
	{
		return SystemDefinitionOverrides;
	}

	void ApplyDefinitionOverrides(FFlecsSystemDefinition& InOutDefinition) const
	{
		ApplySystemDefinitionOverrides(InOutDefinition);
	}

	virtual bool ShouldAutoRegisterFromCDO() const override
	{
		return true;
	}

#if WITH_AUTOMATION_TESTS
	virtual bool ShouldAutoRegisterOnlyForTest() const override
	{
		return true;
	}
#endif // WITH_AUTOMATION_TESTS
	
#if WITH_EDITORONLY_DATA
	virtual bool ShouldShowInSettings() const override
	{
		return false;
	}
#endif // 

	virtual EUnrealFlecsRegistrationScopeType GetRegistrationScopeType() const override
	{
		return EUnrealFlecsRegistrationScopeType::Module;
	}

}; // class UFlecsSystemDefinitionOverrideTestObject

UCLASS()
class UNREALFLECSTESTS_API UFlecsObserverDefinitionOverrideTestObject final : public UFlecsObserverObject
{
	GENERATED_BODY()

public:
	void SetDefinitionOverrides(const FFlecsObserverDefinitionOverrides& InOverrides)
	{
		ObserverDefinitionOverrides = InOverrides;
	}

	FFlecsObserverDefinitionOverrides& GetDefinitionOverrides()
	{
		return ObserverDefinitionOverrides;
	}

	const FFlecsObserverDefinitionOverrides& GetDefinitionOverrides() const
	{
		return ObserverDefinitionOverrides;
	}

	void ApplyDefinitionOverrides(FFlecsObserverDefinition& InOutDefinition) const
	{
		ApplyObserverDefinitionOverrides(InOutDefinition);
	}

	virtual bool ShouldAutoRegisterFromCDO() const override
	{
		return false;
	}
	
#if WITH_EDITORONLY_DATA
	virtual bool ShouldShowInSettings() const override
	{
		return false;
	}
#endif // 

	virtual EUnrealFlecsRegistrationScopeType GetRegistrationScopeType() const override
	{
		return EUnrealFlecsRegistrationScopeType::Module;
	}

}; // class UFlecsObserverDefinitionOverrideTestObject
