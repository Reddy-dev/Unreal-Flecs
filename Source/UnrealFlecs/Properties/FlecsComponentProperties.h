// Elie Wiese-Namir © 2025. All Rights Reserved.

// ReSharper disable CppExpressionWithoutSideEffects
#pragma once

#include <string>
#include <vector>

#include "CoreMinimal.h"
#include "flecs.h"
#include "Standard/robin_hood.h"
#include "SolidMacros/Macros.h"
#include "SolidMacros/Concepts/SolidConcepts.h"
#include "Unlog/Unlog.h"

namespace UnrealFlecs
{
	using FlecsComponentFunctionPtr = std::function<void(flecs::world, flecs::untyped_component)>;
} // namespace UnrealFlecs

struct UNREALFLECS_API FFlecsComponentProperties
{
	std::string Name;
	UScriptStruct* Struct = nullptr;

	uint32 Size = 1;
	uint16 Alignment = 1;

	UnrealFlecs::FlecsComponentFunctionPtr RegistrationFunction;
}; // struct FFlecsComponentProperties

DECLARE_DELEGATE_OneParam(FOnComponentPropertiesRegistered, FFlecsComponentProperties);

struct UNREALFLECS_API FFlecsComponentPropertiesRegistry final
{
	static FFlecsComponentPropertiesRegistry Instance;
public:
	static FFlecsComponentPropertiesRegistry& Get()
	{
		return Instance;
	}

	FORCEINLINE void RegisterComponentProperties(const std::string& Name,
		UScriptStruct* Struct,
		const uint32 Size, const uint16 Alignment,
		const UnrealFlecs::FlecsComponentFunctionPtr* RegistrationFunction)
	{
		UNLOG_CATEGORY_SCOPED(LogFlecsComponentProperties);
		
		ComponentProperties[Name] = FFlecsComponentProperties
		{
			.Name = Name, .Struct = Struct,
			.Size = Size, .Alignment = Alignment,
			.RegistrationFunction = RegistrationFunction ? *RegistrationFunction : nullptr
		};
		
		OnComponentPropertiesRegistered.ExecuteIfBound(ComponentProperties[Name]);
	}

	FORCEINLINE NO_DISCARD bool ContainsComponentProperties(const std::string& Name) const
	{
		return ComponentProperties.contains(Name);
	}

	FORCEINLINE const FFlecsComponentProperties* GetComponentProperties(const std::string& Name) const
	{
		checkf(!Name.empty(), TEXT("Component properties name is empty!"));
		checkf(ComponentProperties.contains(Name), TEXT("Component properties not found!"));
		return &ComponentProperties.at(Name);
	}
	
	robin_hood::unordered_flat_map<std::string, FFlecsComponentProperties> ComponentProperties;
	FOnComponentPropertiesRegistered OnComponentPropertiesRegistered;
	
}; // struct FFlecsComponentPropertiesRegistry

#define INTERNAL_FLECS_REGISTER_FLECS_COMPONENT_LAMBDA(Name, RegistrationFunction) \
	namespace \
	{ \
		struct FFlecs_AutoRegister_##Name \
		{ \
			FFlecs_AutoRegister_##Name() \
			{ \
				FCoreDelegates::OnPostEngineInit.AddLambda([]() \
				{ \
					UnrealFlecs::FlecsComponentFunctionPtr RegistrationFunctionRef = RegistrationFunction; \
					if constexpr (Solid::IsScriptStruct<Name>()) \
					{ \
						FFlecsComponentPropertiesRegistry::Get().RegisterComponentProperties( \
						#Name, TBaseStructure<Name>::Get(), sizeof(Name), alignof(Name), &RegistrationFunctionRef); \
					} \
					else \
					{ \
						FFlecsComponentPropertiesRegistry::Get().RegisterComponentProperties( \
						#Name, nullptr, sizeof(Name), alignof(Name), &RegistrationFunctionRef); \
					} \
				}); \
			} \
		}; \
		static FFlecs_AutoRegister_##Name AutoRegister_##Name; \
	}

#define INTERNAL_FLECS_REGISTER_FLECS_COMPONENT_NO_LAMBDA(Name) \
	namespace \
	{ \
		struct FFlecs_AutoRegister_##Name \
		{ \
			FFlecs_AutoRegister_##Name() \
			{ \
				FCoreDelegates::OnPostEngineInit.AddLambda([]() \
				{ \
					if constexpr (Solid::IsScriptStruct<Name>()) \
					{ \
						FFlecsComponentPropertiesRegistry::Get().RegisterComponentProperties( \
						#Name, TBaseStructure<Name>::Get(), sizeof(Name), alignof(Name), nullptr); \
					} \
					else \
					{ \
						FFlecsComponentPropertiesRegistry::Get().RegisterComponentProperties( \
						#Name, nullptr, sizeof(Name), alignof(Name), nullptr); \
					} \
				}); \
			} \
		}; \
		static FFlecs_AutoRegister_##Name AutoRegister_##Name; \
	}

#define REGISTER_FLECS_COMPONENT_1(Name) \
    INTERNAL_FLECS_REGISTER_FLECS_COMPONENT_NO_LAMBDA(Name)

#define REGISTER_FLECS_COMPONENT_2(Name, RegistrationFunction) \
    INTERNAL_FLECS_REGISTER_FLECS_COMPONENT_LAMBDA(Name, RegistrationFunction)

#define INTERNAL_UNREAL_FLECS_COUNT_ARGS_IMPL(_1, _2, N, ...) N
#define INTERNAL_UNREAL_FLECS_COUNT_ARGS(...) \
    FORCE_EXPAND(INTERNAL_UNREAL_FLECS_COUNT_ARGS_IMPL(__VA_ARGS__, 2, 1))

#define REGISTER_FLECS_COMPONENT(...) \
    FORCE_EXPAND(ECS_CONCAT(REGISTER_FLECS_COMPONENT_, INTERNAL_UNREAL_FLECS_COUNT_ARGS(__VA_ARGS__))(__VA_ARGS__))
	