﻿// Elie Wiese-Namir © 2025. All Rights Reserved.

// ReSharper disable CppExpressionWithoutSideEffects
#pragma once

#include <string>

#include "flecs.h"

#include "CoreMinimal.h"
#include "Misc/CoreDelegates.h"

#include "SolidMacros/Macros.h"
#include "Standard/robin_hood.h"

#include "FlecsTypeTraitsTypes.h"
#include "Entities/FlecsComponentHandle.h"

namespace Unreal::Flecs
{
	using FFlecsComponentFunction = std::function<void(flecs::world, const FFlecsComponentHandle&)>;
} // namespace Unreal::Flecs

// @TODO: Re-Enable Traits
/*struct UNREALFLECS_API FFlecsComponentTypeTraits
{
	EFlecsOnInstantiate OnInstantiate = EFlecsOnInstantiate::Override;
	EFlecsOnDelete OnDelete = EFlecsOnDelete::Remove;
	EFlecsOnDelete OnDeleteTarget = EFlecsOnDelete::Remove;
	
	// Fragmentation Traits
	bool DontFragment = false;
	bool Sparse = DontFragment;

	// Relationship Traits
	bool Relationship = false;
	bool Target = false;
	bool Exclusive = false;
	bool PairIsTag = false;
	bool Symmetric = false;
	bool Reflexive = false;
	bool Acyclic = false;
	bool Traversable = false;
	bool OneOf = false;
	bool Transitive = false;

	// General Type Traits
	bool CanToggle = false;
	bool Singleton = false;
	bool Final = false;
	
}; // struct FFlecsComponentTypeTraits*/

struct UNREALFLECS_API FFlecsComponentProperties
{
	std::string Name;
	TWeakObjectPtr<UScriptStruct> Struct;

	uint32 Size = 1;
	uint16 Alignment = 1;

	Unreal::Flecs::FFlecsComponentFunction RegistrationFunction;
}; // struct FFlecsComponentProperties

DECLARE_MULTICAST_DELEGATE_OneParam(FOnComponentPropertiesRegistered, FFlecsComponentProperties);

struct UNREALFLECS_API FFlecsComponentPropertiesRegistry final
{
	static FFlecsComponentPropertiesRegistry Instance;
	
public:
	static NO_DISCARD FFlecsComponentPropertiesRegistry& Get()
	{
		return Instance;
	}

	void RegisterComponentProperties(const std::string& Name,
	                                 UScriptStruct* Struct,
	                                 const uint32 Size, const uint16 Alignment,
	                                 const Unreal::Flecs::FFlecsComponentFunction& RegistrationFunction);

	NO_DISCARD bool ContainsComponentProperties(const std::string_view Name) const;

	NO_DISCARD bool ContainsComponentProperties(const FString& Name) const;

	NO_DISCARD const FFlecsComponentProperties& GetComponentProperties(const std::string_view Name) const;

	NO_DISCARD const FFlecsComponentProperties& GetComponentProperties(const FString& Name) const;

	robin_hood::unordered_map<std::string, FFlecsComponentProperties> ComponentProperties;
	FOnComponentPropertiesRegistered OnComponentPropertiesRegistered;
	
}; // struct FFlecsComponentPropertiesRegistry

// @TODO: Re-Enable Traits
/*template <typename T>
struct TFlecsTypeTraitsBase
{
	static constexpr EFlecsOnInstantiate OnInstantiate = EFlecsOnInstantiate::Override;
	static constexpr EFlecsOnDelete OnDelete = EFlecsOnDelete::Remove;
	static constexpr EFlecsOnDelete OnDeleteTarget = EFlecsOnDelete::Remove;

	// Fragmentation Traits
	static constexpr bool DontFragment = false;
	static constexpr bool Sparse = DontFragment;

	// Relationship Traits
	static constexpr bool Relationship = false;
	static constexpr bool Target = false;
	static constexpr bool Exclusive = false;
	static constexpr bool PairIsTag = false;
	static constexpr bool Symmetric = false;
	static constexpr bool Reflexive = false;
	static constexpr bool Acyclic = false;
	static constexpr bool Traversable = false;
	static constexpr bool OneOf = false;
	static constexpr bool Transitive = false;

	// General Type Traits
	static constexpr bool CanToggle = false;
	static constexpr bool Singleton = false;
	static constexpr bool Final = false;
	
}; // struct TFlecsTypeTraitsBase*/

/*template <typename T>
struct TFlecsTypeTraits : public TFlecsTypeTraitsBase<T>
{
}; // struct TFlecsTypeTraits*/
	

// @TODO: Consider adding Auto-Registration
// std::function<void(flecs::world, const FFlecsComponentHandle&)>
#define REGISTER_FLECS_COMPONENT(Name, RegistrationFunction) \
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
						#Name, TBaseStructure<Name>::Get(), sizeof(Name), alignof(Name), RegistrationFunction); \
					} \
					else \
					{ \
						FFlecsComponentPropertiesRegistry::Get().RegisterComponentProperties( \
							#Name, nullptr, sizeof(Name), alignof(Name), RegistrationFunction); \
					} \
				}); \
			} \
		}; \
		static FFlecs_AutoRegister_##Name AutoRegister_##Name; \
	}