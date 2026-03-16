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
	
	FORCEINLINE bool operator==(const FFlecsRegisteredComponentEntry& Other) const
	{
		return ComponentDefinition.Name == Other.ComponentDefinition.Name;
	}
	
	FORCEINLINE bool operator!=(const FFlecsRegisteredComponentEntry& Other) const
	{
		return !(*this == Other);
	}
	
public:
	FORCEINLINE FFlecsRegisteredComponentEntry() = default;
	
	FORCEINLINE FFlecsRegisteredComponentEntry(const FFlecsComponentPropertiesDefinition& InDefinition)
		: ComponentDefinition(InDefinition)
	{
	}
	
	FORCEINLINE explicit FFlecsRegisteredComponentEntry(const FString& String)
	{
		ComponentDefinition.Name = String;
	}

	UPROPERTY()
	FFlecsComponentPropertiesDefinition ComponentDefinition; 
	
	UPROPERTY()
	TOptional<FName> ModuleName;
	
}; // struct FFlecsRegisteredComponentEntry

UCLASS()
class UNREALFLECS_API UFlecsTypeRegistryEngineSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	UFlecsTypeRegistryEngineSubsystem();
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	void RegisterAllTypes(const TSolidNotNull<const UFlecsWorld*> InWorld);
	
	void AddRegisteredComponentProperties(const FFlecsComponentPropertiesDefinition& InDefinition);
	
	NO_DISCARD bool IsComponentRegistered(const FString& InComponentName) const;
	NO_DISCARD const FFlecsComponentPropertiesDefinition* GetRegisteredComponentProperties(const FString& InComponentName) const;

	UPROPERTY()
	TSet<FFlecsRegisteredComponentEntry> RegisteredComponentEntries;
	
	TMultiMap<FName, FString> ComponentNameToModuleNameMap;
	
	static TWeakObjectPtr<UFlecsTypeRegistryEngineSubsystem> Singleton;
	
private:
	TArray<FString> SortComponentsByDependencies() const;
	
}; // class UFlecsTypeRegistryEngineSubsystem
