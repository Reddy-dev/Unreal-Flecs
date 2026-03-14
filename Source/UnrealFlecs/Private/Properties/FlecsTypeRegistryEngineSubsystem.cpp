// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Properties/FlecsTypeRegistryEngineSubsystem.h"

#include "Logs/FlecsCategories.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsTypeRegistryEngineSubsystem)

TWeakObjectPtr<UFlecsTypeRegistryEngineSubsystem> UFlecsTypeRegistryEngineSubsystem::Singleton = nullptr;

TArray<FString> UFlecsTypeRegistryEngineSubsystem::SortComponentsByDependencies() const
{
    TMap<FString, TArray<FString>> Graph;
    TMap<FString, const FFlecsComponentPropertiesDefinition*> NameToDefinition;

    for (const FFlecsRegisteredComponentEntry& Entry : RegisteredComponentEntries)
    {
        const FString& Name = Entry.ComponentDefinition.Name;
        NameToDefinition.Add(Name, &Entry.ComponentDefinition);
        Graph.FindOrAdd(Name);
    }
	
    for (const FFlecsRegisteredComponentEntry& Entry : RegisteredComponentEntries)
    {
        const FString& Name = Entry.ComponentDefinition.Name;
        const FFlecsComponentPropertiesDefinition& Definition = Entry.ComponentDefinition;

        TArray<FString> Dependencies;
    	
        auto AddInputDependencies = [&](const TArray<FFlecsQueryGeneratorInput>& Inputs)
        {
            for (const FFlecsQueryGeneratorInput& Input : Inputs)
            {
                FString DependencyName;
                if (UE::Flecs::internal::TryGetDependencyName(Input, DependencyName))
                {
                    Dependencies.Add(DependencyName);
                }
            }
        };

        AddInputDependencies(Definition.DependsOn);
    	
        AddInputDependencies(Definition.WithTypes);
    	
        if (Definition.ChildOf.IsSet())
        {
            FString DependencyName;
            if (UE::Flecs::internal::TryGetDependencyName(Definition.ChildOf.GetValue(), DependencyName))
            {
                Dependencies.Add(DependencyName);
            }
        }
    	
        if (Definition.InheritsFrom.IsSet())
        {
            FString DepName;
            if (UE::Flecs::internal::TryGetDependencyName(Definition.InheritsFrom.GetValue(), DepName))
            {
                Dependencies.Add(DepName);
            }
        }
    	
        for (const FString& Dependency : Dependencies)
        {
            if (!NameToDefinition.Contains(Dependency))
            {
                UE_LOG(LogFlecsCore, Error,
                    TEXT("Component %s depends on %s which is not registered. Skipping dependency."),
                    *Name, *Dependency);
                continue;
            }
        	
            Graph.FindOrAdd(Dependency).Add(Name);
        }
    }
	
    TMap<FString, int32> InDegree;
    for (const TTuple<FString, TArray<FString>>& Pair : Graph)
    {
        InDegree.FindOrAdd(Pair.Key, 0);
        for (const FString& Dependency : Pair.Value)
        {
            InDegree.FindOrAdd(Dependency)++;
        }
    }

    TQueue<FString> ZeroQueue;
    for (const TTuple<FString, int32>& Pair : InDegree)
    {
        if (Pair.Value == 0)
        {
	        ZeroQueue.Enqueue(Pair.Key);
        }
    }

    TArray<FString> Sorted;
    while (!ZeroQueue.IsEmpty())
    {
        FString Node;
        ZeroQueue.Dequeue(Node);
        Sorted.Add(Node);

        for (const FString& Dependency : Graph[Node])
        {
            int32& Count = InDegree[Dependency];
        	--Count;
            if (Count == 0)
            {
	            ZeroQueue.Enqueue(Dependency);
            }
        }
    }
	
    if (Sorted.Num() != Graph.Num())
    {
        UE_LOG(LogFlecsCore, Error,
            TEXT("Cycle detected in component dependencies! Sorted %d of %d components. Check DependsOn/ChildOf/InheritsFrom/WithTypes."),
            Sorted.Num(), Graph.Num());
        
        TArray<FString> Fallback;
        for (const FFlecsRegisteredComponentEntry& Entry : RegisteredComponentEntries)
        {
	        Fallback.Add(Entry.ComponentDefinition.Name);
        }
    	
        return Fallback;
    }

    return Sorted;
}

UFlecsTypeRegistryEngineSubsystem::UFlecsTypeRegistryEngineSubsystem()
{
}

void UFlecsTypeRegistryEngineSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	Singleton = this;
	
	{
		FScopeLock Lock(&UE::Flecs::Private::GetRegisteredComponentsMutex());
		
		TArray<FFlecsComponentPropertiesDefinition>& PendingDefinitions = UE::Flecs::Private::GetPendingRegisteredComponents();
		
		for (const FFlecsComponentPropertiesDefinition& PendingDefinition : PendingDefinitions)
		{
			AddRegisteredComponentProperties(PendingDefinition);
		}
		
		PendingDefinitions.Empty();
	}
}

void UFlecsTypeRegistryEngineSubsystem::RegisterAllTypes(const TSolidNotNull<const UFlecsWorld*> InWorld)
{
	TArray<FString> SortedComponentNames = SortComponentsByDependencies();
	
	for (const FString& ComponentName : SortedComponentNames)
	{
		const FFlecsRegisteredComponentEntry* FoundEntry = RegisteredComponentEntries.Find(FFlecsRegisteredComponentEntry(ComponentName));
		
		if LIKELY_IF(ensureMsgf(FoundEntry, 
			TEXT("UFlecsTypeRegistryEngineSubsystem::RegisterAllTypes: Could not find registered component entry for %s"), *ComponentName))
		{
			const FFlecsComponentPropertiesDefinition& Definition = FoundEntry->ComponentDefinition;
			
			if LIKELY_IF(ensureMsgf(Definition.RegistrationFunction, 
				TEXT("UFlecsTypeRegistryEngineSubsystem::RegisterAllTypes: RegistrationFunction is null for component %s"), *ComponentName))
			{
				Definition.RegistrationFunction(InWorld, Definition);
			}
		}
	}
}

void UFlecsTypeRegistryEngineSubsystem::AddRegisteredComponentProperties(
	const FFlecsComponentPropertiesDefinition& InDefinition)
{
	solid_checkf(!InDefinition.Name.IsEmpty(), 
		TEXT("Invalid component properties definition provided to AddRegisteredComponentProperties: ComponentName is None!"));
	solid_checkf(InDefinition.RegistrationFunction, 
		TEXT("Invalid component properties definition provided to AddRegisteredComponentProperties: RegistrationFunction is null!"));
	
	solid_checkf(!RegisteredComponentEntries.Contains(FFlecsRegisteredComponentEntry(InDefinition.Name)),
		TEXT("Component with name %s is already registered!"), *InDefinition.Name);
	
	RegisteredComponentEntries.Add(FFlecsRegisteredComponentEntry(InDefinition));
}

bool UFlecsTypeRegistryEngineSubsystem::IsComponentRegistered(const FString& InComponentName) const
{
	return RegisteredComponentEntries.Contains(FFlecsRegisteredComponentEntry(InComponentName));
}

const FFlecsComponentPropertiesDefinition* UFlecsTypeRegistryEngineSubsystem::GetRegisteredComponentProperties(
	const FString& InComponentName) const
{
	const FFlecsRegisteredComponentEntry EntryToFind(InComponentName);
	
	if LIKELY_IF(const FFlecsRegisteredComponentEntry* FoundEntry = RegisteredComponentEntries.Find(EntryToFind))
	{
		return &FoundEntry->ComponentDefinition;
	}
	else
	{
		return nullptr;
	}
}
