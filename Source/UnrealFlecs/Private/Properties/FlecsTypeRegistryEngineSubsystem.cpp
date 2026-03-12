// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Properties/FlecsTypeRegistryEngineSubsystem.h"

#include "Logs/FlecsCategories.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsTypeRegistryEngineSubsystem)

UFlecsTypeRegistryEngineSubsystem::UFlecsTypeRegistryEngineSubsystem()
{
}

void UFlecsTypeRegistryEngineSubsystem::RegisterAllTypes(const TSolidNotNull<const UFlecsWorld*> InWorld)
{
	for (const FFlecsComponentPropertiesDefinition& RegisteredType : RegisteredComponentProperties)
	{
		RegisteredType.RegistrationFunction(InWorld, RegisteredType);
	}
}

void UFlecsTypeRegistryEngineSubsystem::AddRegisteredComponentProperties(
	const FFlecsComponentPropertiesDefinition& InDefinition)
{
	solid_checkf(InDefinition.RegistrationFunction, 
		TEXT("Invalid component properties definition provided to AddRegisteredComponentProperties: RegistrationFunction is null!"));
	
	RegisteredComponentProperties.Add(InDefinition);
}

bool UFlecsTypeRegistryEngineSubsystem::IsComponentPropertiesRegisteredForModule(const FString& InModuleName,
	const FFlecsEntityHandle& InComponent) const
{
	if LIKELY_IF(const TArray<FFlecsComponentPropertiesDefinition>* Found = ModuleRegisteredComponentProperties.Find(InModuleName))
	{
		return Found->ContainsByPredicate([&](const FFlecsComponentPropertiesDefinition& Definition)
		{
			return Definition.Name == InComponent.GetSymbol();
		});
	}
	else
	{
		UE_LOGFMT(LogFlecsCore, Warning, 
			"No registered component properties found for module {0} when checking if component {1} is registered for module",
			InModuleName, InComponent.GetSymbol());
	}
		
	return false;
}

const FFlecsComponentPropertiesDefinition* UFlecsTypeRegistryEngineSubsystem::GetRegisteredComponentPropertiesForComponent(
	const FFlecsEntityHandle& InComponent) const
{
	for (const FFlecsComponentPropertiesDefinition& RegisteredType : RegisteredComponentProperties)
	{
		if (RegisteredType.Name == InComponent.GetSymbol())
		{
			return &RegisteredType;
		}
	}
	
	UE_LOGFMT(LogFlecsCore, Warning, 
		"No registered component properties found for component {0} when trying to get registered component properties for component",
		InComponent.GetSymbol());
	
	return nullptr;
}

const FFlecsComponentPropertiesDefinition* UFlecsTypeRegistryEngineSubsystem::GetRegisteredComponentPropertiesForModuleAndComponent(
	const FString& InModuleName,
	const FFlecsEntityHandle& InComponent) const
{
	if LIKELY_IF(const TArray<FFlecsComponentPropertiesDefinition>* Found = ModuleRegisteredComponentProperties.Find(InModuleName))
	{
		if (const FFlecsComponentPropertiesDefinition* FoundDefinition = Found->FindByPredicate([&]
			(const FFlecsComponentPropertiesDefinition& Definition)
		{
			return Definition.Name == InComponent.GetSymbol();
		}))
		{
			return FoundDefinition;
		}
		else
		{
			UE_LOGFMT(LogFlecsCore, Warning, 
				"No registered component properties found for component {0} in module {1} when trying to get registered component properties for module and component",
				InComponent.GetSymbol(), InModuleName);
			return nullptr;
		}
	}
	else
	{
		UE_LOGFMT(LogFlecsCore, Warning, 
			"No registered component properties found for module {0} when trying to get registered component properties for module and component",
			InModuleName);
		return nullptr;
	}
}
