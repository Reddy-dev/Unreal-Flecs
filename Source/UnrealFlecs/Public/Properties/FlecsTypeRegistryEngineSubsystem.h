// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Subsystems/EngineSubsystem.h"

#include "SolidMacros/Macros.h"
#include "Types/SolidNotNull.h"

#include "FlecsComponentProperties.h"

#include "FlecsTypeRegistryEngineSubsystem.generated.h"

class UFlecsWorld;

USTRUCT()
struct UNREALFLECS_API FFlecsRegisteredComponentEntry
{
	GENERATED_BODY()
	
	friend uint32 GetTypeHash(const FFlecsRegisteredComponentEntry& InEntry)
	{
		return GetTypeHash(InEntry.ComponentDefinition.Name);
	}
	
public:
	FORCEINLINE FFlecsRegisteredComponentEntry() = default;
	
	
	
	UPROPERTY()
	FFlecsComponentPropertiesDefinition ComponentDefinition; 
	
}; // struct FFlecsRegisteredComponentEntry

UCLASS()
class UNREALFLECS_API UFlecsTypeRegistryEngineSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	UFlecsTypeRegistryEngineSubsystem();
	
	void RegisterAllTypes(const TSolidNotNull<const UFlecsWorld*> InWorld);
	
	void AddRegisteredComponentProperties(const FFlecsComponentPropertiesDefinition& InDefinition);
	
	NO_DISCARD FORCEINLINE const TArray<FFlecsComponentPropertiesDefinition>& GetRegisteredComponentProperties() const
	{
		return RegisteredComponentProperties;
	}
	
	NO_DISCARD FORCEINLINE const TArray<FFlecsComponentPropertiesDefinition>& GetRegisteredComponentPropertiesForModule(const FString& InModuleName) const
	{
		if (const TArray<FFlecsComponentPropertiesDefinition>* Found = ModuleRegisteredComponentProperties.Find(InModuleName))
		{
			return *Found;
		}
		
		return TArray<FFlecsComponentPropertiesDefinition>();
	}
	
	NO_DISCARD FORCEINLINE bool IsComponentPropertiesRegistered(const FFlecsEntityHandle& InComponent) const
	{
		return RegisteredComponentProperties.ContainsByPredicate([&](const FFlecsComponentPropertiesDefinition& Definition)
		{
			return Definition.Name == InComponent.GetSymbol();
		});
	}
	
	NO_DISCARD bool IsComponentPropertiesRegisteredForModule(const FString& InModuleName, const FFlecsEntityHandle& InComponent) const;
	
	NO_DISCARD const FFlecsComponentPropertiesDefinition* GetRegisteredComponentPropertiesForComponent(const FFlecsEntityHandle& InComponent) const;
	NO_DISCARD const FFlecsComponentPropertiesDefinition* GetRegisteredComponentPropertiesForModuleAndComponent(const FString& InModuleName, const FFlecsEntityHandle& InComponent) const;

	UPROPERTY()
	TArray<FFlecsComponentPropertiesDefinition> RegisteredComponentProperties;
	
	TMap<FString /* ModuleName */, TArray<FFlecsComponentPropertiesDefinition>> ModuleRegisteredComponentProperties;
	
	
}; // class UFlecsTypeRegistryEngineSubsystem
