// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Properties/FlecsComponentProperties.h"

FFlecsComponentPropertiesRegistry FFlecsComponentPropertiesRegistry::Instance;

void FFlecsComponentPropertiesRegistry::RegisterComponentProperties(const FString& Name, UScriptStruct* Struct,
	const uint32 Size, const uint16 Alignment, const UE::Flecs::FFlecsComponentFunction& RegistrationFunction)
{
	solid_checkf(!Name.IsEmpty(), TEXT("Component properties name is empty!"));
		
	ComponentProperties[Name] = FFlecsComponentPropertiesDefinition
	{
		.Name = Name,
		.ScriptStruct = Struct,
		.Size = Size, .Alignment = Alignment,
		.RegistrationFunction = RegistrationFunction
	};
		
	OnComponentPropertiesRegistered.Broadcast(ComponentProperties[Name]);
}

bool FFlecsComponentPropertiesRegistry::ContainsComponentProperties(const FString& Name) const
{
	return ComponentProperties.Contains(StringCast<char>(*Name).Get());
}

const FFlecsComponentPropertiesDefinition& FFlecsComponentPropertiesRegistry::GetComponentProperties(const FString& Name) const
{
	solid_checkf(!Name.IsEmpty(), TEXT("Component properties name is empty!"));
	solid_checkf(ComponentProperties.Contains(StringCast<char>(*Name).Get()),
	             TEXT("Component properties not found!"));
	
	return ComponentProperties[StringCast<char>(*Name).Get()];
}
