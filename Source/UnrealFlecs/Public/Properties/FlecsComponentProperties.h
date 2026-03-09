// Elie Wiese-Namir © 2025. All Rights Reserved.

// ReSharper disable CppExpressionWithoutSideEffects
#pragma once

#include <string>

#include "flecs.h"

#include "CoreMinimal.h"
#include "Misc/CoreDelegates.h"

#include "SolidMacros/Macros.h"
#include "Standard/robin_hood.h"
#include "Types/SolidCppStructOps.h"

#include "Entities/FlecsComponentHandle.h"

#include "FlecsComponentProperties.generated.h"

UENUM(BlueprintType)
enum class EFlecsOnInstantiate : uint8
{
	Override,
	Inherit,
	DontInherit,
	Count UMETA(Hidden)
}; // enum class EFlecsOnInstantiate
ENUM_RANGE_BY_COUNT(EFlecsOnInstantiate, EFlecsOnInstantiate::Count);

UENUM(BlueprintType)
enum class EFlecsOnDelete : uint8
{
	Remove,
	Delete,
	Panic,
	Count UMETA(Hidden)
}; // enum class EFlecsOnDelete
ENUM_RANGE_BY_COUNT(EFlecsOnDelete, EFlecsOnDelete::Count);

namespace UE::Flecs
{
	using FFlecsComponentFunction = std::function<void(flecs::world, const FFlecsComponentHandle&)>;

	namespace internal
	{
		template <typename T>
		FORCEINLINE UScriptStruct* GetScriptStructIf()
		{
			if constexpr (Solid::IsScriptStruct<T>())
			{
				return TBaseStructure<T>::Get();
			}
			else
			{
				return nullptr;
			}
		}
		
	} // namespace internal
	
} // namespace UE::Flecs

// @TODO: rename this due to it being too generic and risks name collisions
USTRUCT()
struct UNREALFLECS_API FFlecsComponentPropertiesDefinition
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	uint8 bAutoRegister : 1 = true;
	
	UPROPERTY()
	FString Name;
	
	UPROPERTY()
	TWeakObjectPtr<const UStruct> MetaType;
	
	UPROPERTY()
	uint32 Size = 0;
	
	UPROPERTY()
	uint16 Alignment = 0;
	
	UPROPERTY()
	EFlecsOnInstantiate OnInstantiate = EFlecsOnInstantiate::Override;
	
	UPROPERTY()
	EFlecsOnDelete OnDelete = EFlecsOnDelete::Remove;
	
	UPROPERTY()
	EFlecsOnDelete OnDeleteTarget = EFlecsOnDelete::Remove;
	
	UPROPERTY()
	uint32 bSingleton : 1 = false;
	
	UPROPERTY()
	uint32 bTrait : 1 = false;
	
	UPROPERTY()
	uint32 bSparse : 1 = false;
	
	UPROPERTY()
	uint32 bDontFragment : 1 = false;
	
	UPROPERTY()
	uint32 bRelationship : 1 = false;
	
	UPROPERTY()
	uint32 bTarget : 1 = false;
	
	UPROPERTY()
	uint32 bPairIsTag : 1 = false;
	
	UPROPERTY()
	uint32 bReflexive : 1 = false;
	
	UPROPERTY()
	uint32 bAcyclic : 1 = false;
	
	UPROPERTY()
	uint32 bTraversable : 1 = false;
	
	UPROPERTY()
	uint32 bTransitive : 1 = false;
	
	UPROPERTY()
	uint32 bOneOf : 1 = false;
	
	UPROPERTY()
	uint32 bCanToggle : 1 = false;
	
	UPROPERTY()
	uint32 bInheritable : 1 = false;
	
	UPROPERTY()
	uint32 bFinal : 1 = false;
	
	UPROPERTY()
	uint32 bWithAddReferencedObjects : 1 = false;
	
}; // struct FFlecsComponentProperties

template <typename T>
struct TFlecsComponentTraitsBase
{
public:
	static constexpr bool AutoRegister = true;
	
	static constexpr EFlecsOnInstantiate OnInstantiate = EFlecsOnInstantiate::Override;	
	static constexpr EFlecsOnDelete OnDelete = EFlecsOnDelete::Remove;
	static constexpr EFlecsOnDelete OnDeleteTarget = EFlecsOnDelete::Remove;
	
	static constexpr bool Singleton = false;
	static constexpr bool Trait = false;
	
	static constexpr bool Sparse = false;
	static constexpr bool DontFragment = false;
	
	static constexpr bool Relationship = false;
	static constexpr bool Target = false;
	static constexpr bool PairIsTag = false;
	static constexpr bool Reflexive = false;
	static constexpr bool Acyclic = false;
	static constexpr bool Traversable = false;
	static constexpr bool Transitive = false;
	static constexpr bool OneOf = false;
	
	static constexpr bool CanToggle = false;
	static constexpr bool Inheritable = false;
	static constexpr bool Final = false;
	
	static constexpr bool WithAddReferencedObjects = false;
	
	using WithTypes = TTuple<>;
	using ChildOf = void;
	using DependsOn = TTuple<>;
	
	static void PreRegister(const FFlecsComponentHandle& ComponentHandle) {}
	static void PostRegister(const FFlecsComponentHandle& ComponentHandle) {}
	
}; // struct TFlecsComponentTraitsBase

template <typename T>
struct TFlecsComponentTraits : public TFlecsComponentTraitsBase<T>
{
	
}; // struct TFlecsComponentTraits

DECLARE_MULTICAST_DELEGATE_OneParam(FOnComponentPropertiesRegistered, FFlecsComponentPropertiesDefinition);

struct UNREALFLECS_API FFlecsComponentPropertiesRegistry final
{
	static FFlecsComponentPropertiesRegistry Instance;
	
public:
	static NO_DISCARD FFlecsComponentPropertiesRegistry& Get()
	{
		return Instance;
	}

	void RegisterComponentProperties(const FString& Name,
	                                 UScriptStruct* Struct,
	                                 const uint32 Size, const uint16 Alignment,
	                                 const UE::Flecs::FFlecsComponentFunction& RegistrationFunction);

	NO_DISCARD bool ContainsComponentProperties(const FString& Name) const;

	NO_DISCARD const FFlecsComponentPropertiesDefinition& GetComponentProperties(const FString& Name) const;

	TMap<FString, FFlecsComponentPropertiesDefinition> ComponentProperties;
	FOnComponentPropertiesRegistered OnComponentPropertiesRegistered;
	
}; // struct FFlecsComponentPropertiesRegistry

template <typename TComponent>
struct TFlecsComponentRegistrationHelper
{
	using FTraits = TFlecsComponentTraits<TComponent>;
	
public:
	FORCEINLINE TFlecsComponentRegistrationHelper(const FString& InName, UE::Flecs::FFlecsComponentFunction InRegistrationFunction)
	{
		
	}
	
}; // struct TFlecsComponentRegstrationHelper

#define INTERNAL_REGISTER_FLECS_COMPONENT_IMPL(Name) \
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
							#Name, UE::Flecs::internal::GetScriptStructIf<Name>(), \
							sizeof(Name), alignof(Name), RegistrationFunction); \
						\
						if constexpr (std::is_move_constructible<Name>::value || std::is_move_assignable<Name>::value) \
						{ \
							FSolidMoveableStructRegistry::Get().RegisterMovableScriptStruct<Name>(); \
						} \
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

// @TODO: Consider adding Auto-Registration
#define REGISTER_FLECS_COMPONENT(ComponentType) \
	INTERNAL_REGISTER_FLECS_COMPONENT_IMPL(ComponentType)