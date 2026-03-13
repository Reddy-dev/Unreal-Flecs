// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Properties/FlecsTypeRegistryEngineSubsystem.h"

#include "Logs/FlecsCategories.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsTypeRegistryEngineSubsystem)

TWeakObjectPtr<UFlecsTypeRegistryEngineSubsystem> UFlecsTypeRegistryEngineSubsystem::Singleton = nullptr;

TArray<FString> UFlecsTypeRegistryEngineSubsystem::SortComponentsByDependencies() const
{
    TMap<FString, TArray<FString>> Graph;
    TMap<FString, const FFlecsComponentPropertiesDefinition*> NameToDef;

    for (const FFlecsRegisteredComponentEntry& Entry : RegisteredComponentEntries)
    {
        const FString& Name = Entry.ComponentDefinition.Name;
        NameToDef.Add(Name, &Entry.ComponentDefinition);
        Graph.FindOrAdd(Name); // ensure entry exists
    }
	
    for (const FFlecsRegisteredComponentEntry& Entry : RegisteredComponentEntries)
    {
        const FString& Name = Entry.ComponentDefinition.Name;
        const FFlecsComponentPropertiesDefinition& Def = Entry.ComponentDefinition;

        TArray<FString> Deps;
    	
        auto AddInputDeps = [&](const TArray<FFlecsQueryGeneratorInput>& Inputs)
        {
            for (const FFlecsQueryGeneratorInput& Input : Inputs)
            {
                FString DepName;
                if (UE::Flecs::internal::TryGetDependencyName(Input, DepName))
                {
                    Deps.Add(DepName);
                }
            }
        };

        AddInputDeps(Def.DependsOn);
    	
        AddInputDeps(Def.WithTypes);
    	
        if (Def.ChildOf.IsSet())
        {
            FString DependencyName;
            if (UE::Flecs::internal::TryGetDependencyName(Def.ChildOf.GetValue(), DependencyName))
            {
                Deps.Add(DependencyName);
            }
        }

        // InheritsFrom (optional)
        if (Def.InheritsFrom.IsSet())
        {
            FString DepName;
            if (UE::Flecs::internal::TryGetDependencyName(Def.InheritsFrom.GetValue(), DepName))
            {
                Deps.Add(DepName);
            }
        }

        // Add edges from Name to each Dep
        for (const FString& Dep : Deps)
        {
            if (!NameToDef.Contains(Dep))
            {
                UE_LOG(LogFlecsCore, Error,
                    TEXT("Component %s depends on %s which is not registered. Skipping dependency."),
                    *Name, *Dep);
                continue;
            }
        	
            Graph.FindOrAdd(Dep).Add(Name);
        }
    }
	
    TMap<FString, int32> InDegree;
    for (const auto& Pair : Graph)
    {
        InDegree.FindOrAdd(Pair.Key, 0);
        for (const FString& Dep : Pair.Value)
        {
            InDegree.FindOrAdd(Dep)++;
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

        for (const FString& Dep : Graph[Node])
        {
            int32& Count = InDegree[Dep];
            if (--Count == 0)
            {
	            ZeroQueue.Enqueue(Dep);
            }
        }
    }

    // Check for cycles
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
