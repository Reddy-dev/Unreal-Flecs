// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Properties/FlecsComponentProperties.h"

#include "Properties/FlecsTypeRegistryEngineSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsComponentProperties)

namespace UE::Flecs::Private
{
	FCriticalSection& GetRegisteredComponentsMutex()
	{
		static FCriticalSection Mutex;
		return Mutex;
	}
	
	TArray<FFlecsComponentPropertiesDefinition>& GetPendingRegisteredComponents()
	{
		static TArray<FFlecsComponentPropertiesDefinition> PendingDefinitions;
		return PendingDefinitions;
	}
	
	void AddRegisteredComponentProperties_Static(const FFlecsComponentPropertiesDefinition& InDefinition)
	{
		solid_checkf(!InDefinition.Name.IsEmpty(),
			TEXT("Invalid component properties definition provided to RegisterComponentPropertiesStatic: Name is empty!"));

		solid_checkf(InDefinition.RegistrationFunction,
			TEXT("Invalid component properties definition provided to RegisterComponentPropertiesStatic: RegistrationFunction is null!"));

		FScopeLock Lock(&GetRegisteredComponentsMutex());

		if (UFlecsTypeRegistryEngineSubsystem::Singleton.IsValid())
		{
			UFlecsTypeRegistryEngineSubsystem::Singleton.Get()->AddRegisteredComponentProperties(InDefinition);
			return;
		}

		TArray<FFlecsComponentPropertiesDefinition>& PendingDefinitions = GetPendingRegisteredComponents();

		const bool bAlreadyPending = PendingDefinitions.ContainsByPredicate(
			[&InDefinition](const FFlecsComponentPropertiesDefinition& ExistingDefinition)
			{
				return ExistingDefinition.Name == InDefinition.Name;
			});

		// if you are hitting this, you most likely have REGISTER_FLECS_COMPONENT in a header file or inl file
		solid_checkf(!bAlreadyPending,
			TEXT("Component with name %s is already pending registration!"),
			*InDefinition.Name);

		PendingDefinitions.Add(InDefinition);
	}
	
} // namespace UE::Flecs::Private