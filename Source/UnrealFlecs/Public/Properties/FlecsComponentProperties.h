// Elie Wiese-Namir © 2025. All Rights Reserved.

// ReSharper disable CppExpressionWithoutSideEffects
#pragma once

#include <string>

#include "flecs.h"

#include "Misc/CoreDelegates.h"

#include "SolidMacros/Macros.h"
#include "Standard/robin_hood.h"

#include "Logs/FlecsCategories.h"
#include "Worlds/FlecsWorld.h"
#include "Entities/FlecsComponentHandle.h"
#include "Components/FlecsAddReferencedObjectsTrait.h"
#include "General/UnrealFlecsRegistrationScopeType.h"
#include "Properties/FlecsComponentRegistrationHooks.h"
#include "Queries/Generator/FlecsQueryGeneratorInput.h"
#include "Queries/Generator/FlecsQueryGeneratorInputType.h"

#include "FlecsComponentProperties.generated.h"

struct FFlecsComponentPropertiesDefinition;

UENUM(BlueprintType)
enum class EFlecsOnInstantiate : uint8
{
	Override,
	Inherit,
	DontInherit,
	Count UMETA(Hidden)
}; // enum class EFlecsOnInstantiate
ENUM_RANGE_BY_COUNT(EFlecsOnInstantiate, EFlecsOnInstantiate::Count);

template <flecs::on_instantiate InTrait>
consteval EFlecsOnInstantiate ConvertToFlecsOnInstantiate()
{
	if constexpr (InTrait == flecs::on_instantiate::override)
	{
		return EFlecsOnInstantiate::Override;
	}
	else if constexpr (InTrait == flecs::on_instantiate::inherit)
	{
		return EFlecsOnInstantiate::Inherit;
	}
	else if constexpr (InTrait == flecs::on_instantiate::dont_inherit)
	{
		return EFlecsOnInstantiate::DontInherit;
	}
	else
	{
		static_assert(false, "Invalid flecs::on_instantiate trait value");
	}

	return EFlecsOnInstantiate::Override;
}

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
	using FFlecsComponentRegistrationFunction = void(*)(const TSolidNotNull<const UFlecsWorld*>, const FFlecsComponentPropertiesDefinition&);
	using FFlecsComponentPropertiesFunction = void(*)(const TSolidNotNull<const UFlecsWorld*>, const FFlecsComponentHandle&, const FFlecsComponentPropertiesDefinition&);
	using FFlecsSetSingletonDefaultValueFunction = void(*)(const TSolidNotNull<const UFlecsWorld*>, void* OutValue);

	namespace internal
	{
		template <typename T>
		NO_DISCARD FORCEINLINE UField* GetMetaTypeIf()
		{
			return nullptr;
		}

		template <Solid::TScriptStructConcept T>
		NO_DISCARD FORCEINLINE UField* GetMetaTypeIf()
		{
			return TBaseStructure<T>::Get();
		}

		template <Solid::TStaticClassConcept T>
		NO_DISCARD FORCEINLINE UField* GetMetaTypeIf()
		{
			return StaticClass<T>();
		}

		template <Solid::TStaticEnumConcept T>
		NO_DISCARD FORCEINLINE UField* GetMetaTypeIf()
		{
			return StaticEnum<T>();
		}

		template <typename TTuple, typename TFunction, int32... Indices>
		void ForEachInTupleImpl(TFunction&& Function, TIntegerSequence<int32, Indices...>)
		{
			(Function.template operator()<typename TTupleElement<Indices, TTuple>::Type>(), ...);
		}

		template <typename TTuple, typename TFunction>
		void ForEachInTuple(TFunction&& Function)
		{
			ForEachInTupleImpl<TTuple>(Forward<TFunction>(Function), TMakeIntegerSequence<int32, TTupleArity<TTuple>::Value>{});
		}

		NO_DISCARD FORCEINLINE bool TryGetDependencyName(const FFlecsQueryGeneratorInput& Input, FString& OutName)
		{
			OutName.Reset();

			if (!Input.First.IsValid())
			{
				return false;
			}

			if LIKELY_IF(const FFlecsQueryGeneratorInputType_CPPType* CPPType = Input.First.GetPtr<FFlecsQueryGeneratorInputType_CPPType>())
			{
				OutName = CPPType->SymbolString;
				return !OutName.IsEmpty();
			}

			if LIKELY_IF(const FFlecsQueryGeneratorInputType_CPPEnum* CPPEnum = Input.First.GetPtr<FFlecsQueryGeneratorInputType_CPPEnum>())
			{
				OutName = CPPEnum->SymbolString;
				return !OutName.IsEmpty();
			}

			if LIKELY_IF(const FFlecsQueryGeneratorInputType_ScriptStruct* ScriptStructType = Input.First.GetPtr<FFlecsQueryGeneratorInputType_ScriptStruct>())
			{
				if (ScriptStructType->ScriptStruct)
				{
					OutName = ScriptStructType->ScriptStruct->GetName();
					return true;
				}
			}

			if LIKELY_IF(const FFlecsQueryGeneratorInputType_ScriptEnum* ScriptEnumType = Input.First.GetPtr<FFlecsQueryGeneratorInputType_ScriptEnum>())
			{
				if (ScriptEnumType->ScriptEnum)
				{
					OutName = ScriptEnumType->ScriptEnum->GetName();
					return true;
				}
			}

			return false;
		}

	} // namespace internal

} // namespace UE::Flecs

/**
 * Compile-time registration properties for an Unreal-Flecs component type.
 *
 * Specialize TFlecsComponentTraits with FLECS_COMPONENT_TRAITS and override only the
 * properties required by the component. Every component must also be registered with
 * REGISTER_FLECS_COMPONENT in a source file.
 *
 * Flecs traits are applied to the component entity when it is registered. See the
 * Flecs component-traits manual for their behavior and limitations:
 * https://www.flecs.dev/flecs/ComponentTraits.html
 *
 * @code
 * FLECS_COMPONENT_TRAITS(FHealthComponent)
 * {
 *     static constexpr bool Replicate = true;
 *     static constexpr bool CanToggle = true;
 * };
 *
 * // In one source file:
 * REGISTER_FLECS_COMPONENT(FHealthComponent);
 * @endcode
 */
template <typename T>
struct TFlecsComponentTraitsBase
{
public:
	/** Automatically register the component type during Unreal-Flecs startup. */
	static constexpr bool AutoRegister = true;

	/**
	 * Components and tags added to the registered component entity through Flecs' With trait.
	 * @see https://www.flecs.dev/flecs/ComponentTraits.html#with-trait
	 */
	using WithTypes = TTuple<>;

	/** Optional parent type for the registered component entity. */
	using ChildOf = void;

	/** Types that the registered component entity depends on through flecs::DependsOn. */
	using DependsOn = TTuple<>;
	
	/** Optional base type applied to the registered component entity through flecs::IsA. */
	using InheritsFrom = void;
	
	/** Additional typed traits added to the registered component entity. */
	using CustomTraits = TTuple<>;

	/**
	 * Controls whether a component is copied, inherited, or omitted when an IsA instance is created.
	 * The native `static constexpr auto on_instantiate` Flecs trait is detected automatically.
	 * @see https://www.flecs.dev/flecs/ComponentTraits.html#oninstantiate-trait
	 */
	static constexpr EFlecsOnInstantiate OnInstantiate = ConvertToFlecsOnInstantiate<flecs::on_instantiate_trait<T>::value>();

	/**
	 * Cleanup action when the component entity is deleted.
	 * @see https://www.flecs.dev/flecs/ComponentTraits.html#cleanup-traits
	 */
	static constexpr EFlecsOnDelete OnDelete = EFlecsOnDelete::Remove;

	/**
	 * Cleanup action when this relationship's target is deleted.
	 * @see https://www.flecs.dev/flecs/ComponentTraits.html#cleanup-traits
	 */
	static constexpr EFlecsOnDelete OnDeleteTarget = EFlecsOnDelete::Remove;

	/**
	 * Marks the component as a Flecs singleton.
	 * @see https://www.flecs.dev/flecs/ComponentTraits.html#singleton-trait
	 */
	static constexpr bool Singleton = false;
	
	/** Calls SetSingletonDefaultValue after registration and ensures that a singleton value exists. */
	static constexpr bool CustomSingletonValue = false;

	/**
	 * Marks this component as a trait that may be added to other component entities.
	 * @see https://www.flecs.dev/flecs/ComponentTraits.html#trait-trait
	 */
	static constexpr bool Trait = false;

	/**
	 * Stores values outside archetype tables without fragmenting those tables.
	 * The native `static constexpr bool dont_fragment` Flecs trait is detected automatically.
	 * @see https://www.flecs.dev/flecs/ComponentTraits.html#dontfragment-trait
	 */
	static constexpr bool DontFragment = flecs::dont_fragment<T>::value;

	/**
	 * Stores values in sparse storage while retaining the component in entity table types.
	 * The native `static constexpr bool sparse` Flecs trait is detected automatically.
	 * @see https://www.flecs.dev/flecs/ComponentTraits.html#sparse-trait
	 */
	static constexpr bool Sparse = flecs::sparse<T>::value;

	/**
	 * Constrains this component to the relationship position of a pair.
	 * @see https://www.flecs.dev/flecs/ComponentTraits.html#relationship-trait
	 */
	static constexpr bool Relationship = false;

	/**
	 * Constrains this component to the target position of a pair.
	 * @see https://www.flecs.dev/flecs/ComponentTraits.html#target-trait
	 */
	static constexpr bool Target = false;

	/**
	 * Prevents pairs with this relationship from storing data from either pair element.
	 * @see https://www.flecs.dev/flecs/ComponentTraits.html#pairistag-trait
	 */
	static constexpr bool PairIsTag = false;

	/**
	 * Makes (Relationship, Entity) implicitly true for an entity with this relationship.
	 * @see https://www.flecs.dev/flecs/ComponentTraits.html#reflexive-trait
	 */
	static constexpr bool Reflexive = false;

	/**
	 * Declares that this relationship must not contain cycles.
	 * @see https://www.flecs.dev/flecs/ComponentTraits.html#acyclic-trait
	 */
	static constexpr bool Acyclic = false;

	/**
	 * Allows queries and events to traverse this relationship.
	 * @see https://www.flecs.dev/flecs/ComponentTraits.html#traversable-trait
	 */
	static constexpr bool Traversable = false;

	/**
	 * Enables transitive query matching for this relationship.
	 * @see https://www.flecs.dev/flecs/ComponentTraits.html#transitive-trait
	 */
	static constexpr bool Transitive = false;

	/**
	 * Requires targets of this relationship to be children of the relationship entity.
	 * @see https://www.flecs.dev/flecs/ComponentTraits.html#oneof-trait
	 */
	static constexpr bool OneOf = false;

	/**
	 * Allows component instances to be enabled and disabled without removing them.
	 * @see https://www.flecs.dev/flecs/ComponentTraits.html#cantoggle-trait
	 */
	static constexpr bool CanToggle = false;

	/**
	 * Marks the component as inheritable so queries account for IsA inheritance.
	 * @see https://www.flecs.dev/flecs/ComponentTraits.html#inheritable-trait
	 */
	static constexpr bool Inheritable = false;

	/**
	 * Prevents the component entity from being used as an IsA target.
	 * @see https://www.flecs.dev/flecs/ComponentTraits.html#final-trait
	 */
	static constexpr bool Final = false;

	/** Registers this component with the configured Unreal-Flecs replication backend. */
	static constexpr bool Replicate = false;

	/** Enables Unreal UObject reference collection for values stored in this component. */
	static constexpr bool WithAddReferencedObjects = false;

	/** Registers reflected USTRUCT members with Flecs meta when T is a UScriptStruct. */
	static constexpr bool RegisterMemberProperties = true;

	/** Requests an ID from Flecs' low component-ID range. */
	static constexpr bool UseLowId = true;
	
	/** Selects the module, plugin, explicit scope, or root scope used for registration. */
	static constexpr EUnrealFlecsRegistrationScopeType RegistrationScopeType = EUnrealFlecsRegistrationScopeType::Unset;

	/** Returns the explicit registration scope name, when one is required. */
	static FString GetRegistrationScopeName()
	{
		return "";
	}

	/** Returns auto-registered type names that must be registered before this component. */
	static const TArray<FString>& CustomTypeDependencies()
	{
		static const TArray<FString> EmptyArray;
		return EmptyArray;
	}

	/** Called before built-in properties are applied to an auto-registered typed component. */
	static void PrePropertiesRegistration(const FFlecsComponentHandle& ComponentHandle) {}

	/** Called after all built-in and custom properties are applied. */
	static void PostRegister(const FFlecsComponentHandle& ComponentHandle) {}

	/** Initializes the singleton value when CustomSingletonValue is enabled. */
	static void SetSingletonDefaultValue(const TSolidNotNull<const UFlecsWorld*> World, OUT T& OutValue)
	{
	}

}; // struct TFlecsComponentTraitsBase

template <typename T>
struct TFlecsComponentTraits : public TFlecsComponentTraitsBase<T>
{

}; // struct TFlecsComponentTraits

#define FLECS_COMPONENT_TRAITS(ComponentType) \
	template <> \
	struct TFlecsComponentTraits<ComponentType> : public TFlecsComponentTraitsBase<ComponentType>

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
	uint32 bCustomSingletonValue : 1 = false;

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
	uint32 bReplicate : 1 = false;

	UPROPERTY()
	uint32 bWithAddReferencedObjects : 1 = false;

	// Only matters if the component is a UScriptStruct Type
	UPROPERTY()
	uint32 bRegisterMemberProperties : 1 = false;
	
	UPROPERTY()
	uint32 bUseLowId : 1 = true;

	UPROPERTY()
	TArray<FFlecsQueryGeneratorInput> WithTypes;

	UPROPERTY()
	TOptional<FFlecsQueryGeneratorInput> ChildOf;

	UPROPERTY()
	TArray<FFlecsQueryGeneratorInput> DependsOn;

	UPROPERTY()
	TOptional<FFlecsQueryGeneratorInput> InheritsFrom;
	
	UPROPERTY()
	TArray<FFlecsQueryGeneratorInput> CustomTraits;

	UPROPERTY()
	TArray<FString> CustomTypeDependencies;
	
	UPROPERTY()
	EUnrealFlecsRegistrationScopeType RegistrationScopeType;

	UPROPERTY()
	FString RegistrationScopeName;

	UE::Flecs::FFlecsComponentRegistrationFunction RegistrationFunction;
	UE::Flecs::FFlecsComponentPropertiesFunction PropertiesFunction;
	UE::Flecs::FFlecsSetSingletonDefaultValueFunction SetSingletonDefaultValueFunction;

	template <typename T>
	static NO_DISCARD FORCEINLINE FFlecsComponentPropertiesDefinition Make()
	{
		FFlecsComponentPropertiesDefinition Definition
		{
			.bAutoRegister = TFlecsComponentTraits<T>::AutoRegister,
			.Name = FString(nameof(T).data()),
			.Size = sizeof(T),
			.Alignment = alignof(T),
			.OnInstantiate = TFlecsComponentTraits<T>::OnInstantiate,
			.OnDelete = TFlecsComponentTraits<T>::OnDelete,
			.OnDeleteTarget = TFlecsComponentTraits<T>::OnDeleteTarget,
			.bSingleton = TFlecsComponentTraits<T>::Singleton,
			.bCustomSingletonValue = TFlecsComponentTraits<T>::CustomSingletonValue,
			.bTrait = TFlecsComponentTraits<T>::Trait,
			.bSparse = TFlecsComponentTraits<T>::Sparse,
			.bDontFragment = TFlecsComponentTraits<T>::DontFragment,
			.bRelationship = TFlecsComponentTraits<T>::Relationship,
			.bTarget = TFlecsComponentTraits<T>::Target,
			.bPairIsTag = TFlecsComponentTraits<T>::PairIsTag,
			.bReflexive = TFlecsComponentTraits<T>::Reflexive,
			.bAcyclic = TFlecsComponentTraits<T>::Acyclic,
			.bTraversable = TFlecsComponentTraits<T>::Traversable,
			.bTransitive = TFlecsComponentTraits<T>::Transitive,
			.bOneOf = TFlecsComponentTraits<T>::OneOf,
			.bCanToggle = TFlecsComponentTraits<T>::CanToggle,
			.bInheritable = TFlecsComponentTraits<T>::Inheritable,
			.bFinal = TFlecsComponentTraits<T>::Final,
			.bReplicate = TFlecsComponentTraits<T>::Replicate,
			.bWithAddReferencedObjects = TFlecsComponentTraits<T>::WithAddReferencedObjects,
			.bRegisterMemberProperties = TFlecsComponentTraits<T>::RegisterMemberProperties,
			.bUseLowId = TFlecsComponentTraits<T>::UseLowId,
			.RegistrationScopeType = TFlecsComponentTraits<T>::RegistrationScopeType,
			.RegistrationScopeName = TFlecsComponentTraits<T>::GetRegistrationScopeName(),
		};

		UE::Flecs::internal::ForEachInTuple<typename TFlecsComponentTraits<T>::WithTypes>([&Definition]
			<Solid::TScriptStructConcept TWithType>()
		{
			using FTypeValue = TWithType;

			FFlecsQueryGeneratorInput Input;
			Input.bPair = false;
			Input.First.InitializeAs<FFlecsQueryGeneratorInputType_CPPType>();
			Input.First.GetMutable<FFlecsQueryGeneratorInputType_CPPType>().SymbolString = FString(nameof(FTypeValue).data());

			Definition.WithTypes.Add(Input);
		});

		if constexpr (!std::is_same_v<void, typename TFlecsComponentTraits<T>::InheritsFrom>)
		{
			using FTypeValue = TFlecsComponentTraits<T>::InheritsFrom;

			FFlecsQueryGeneratorInput Input;
			Input.bPair = false;
			Input.First.InitializeAs<FFlecsQueryGeneratorInputType_CPPType>();
			Input.First.GetMutable<FFlecsQueryGeneratorInputType_CPPType>().SymbolString = FString(nameof(FTypeValue).data());

			Definition.InheritsFrom = Input;
		}

		UE::Flecs::internal::ForEachInTuple<typename TFlecsComponentTraits<T>::DependsOn>([&Definition]
			<Solid::TScriptStructConcept TDependsOn>(TDependsOn Type)
		{
			using FTypeValue = TDependsOn;

			FFlecsQueryGeneratorInput Input;
			Input.bPair = false;
			Input.First.InitializeAs<FFlecsQueryGeneratorInputType_CPPType>();
			Input.First.GetMutable<FFlecsQueryGeneratorInputType_CPPType>().SymbolString = FString(nameof(FTypeValue).data());

			Definition.DependsOn.Add(Input);
		});

		if constexpr (!std::is_same_v<void, typename TFlecsComponentTraits<T>::ChildOf>)
		{
			UE::Flecs::internal::ForEachInTuple<typename TFlecsComponentTraits<T>::ChildOf>([&Definition]
				<Solid::TScriptStructConcept TChildOf>(TChildOf Type)
			{
				using FTypeValue = TChildOf;

				FFlecsQueryGeneratorInput Input;
				Input.bPair = false;
				Input.First.InitializeAs<FFlecsQueryGeneratorInputType_CPPType>();
				Input.First.GetMutable<FFlecsQueryGeneratorInputType_CPPType>().SymbolString = FString(nameof(FTypeValue).data());

				Definition.ChildOf = Input;
			});
		}
		
		UE::Flecs::internal::ForEachInTuple<typename TFlecsComponentTraits<T>::CustomTraits>([&Definition]
			<Solid::TScriptStructConcept TCustomTraitType>()
		{
			using FTypeValue = TCustomTraitType;

			FFlecsQueryGeneratorInput Input;
			Input.bPair = false;
			Input.First.InitializeAs<FFlecsQueryGeneratorInputType_CPPType>();
			Input.First.GetMutable<FFlecsQueryGeneratorInputType_CPPType>().SymbolString = FString(nameof(FTypeValue).data());

			Definition.CustomTraits.Add(Input);
		});


		const TArray<FString> CustomTypeDependencies = TFlecsComponentTraits<T>::CustomTypeDependencies();
		Definition.CustomTypeDependencies.Append(CustomTypeDependencies);

		Definition.PropertiesFunction = [](const TSolidNotNull<const UFlecsWorld*> InFlecsWorld, const FFlecsComponentHandle& ComponentHandle, const FFlecsComponentPropertiesDefinition& ComponentProperties)
		{
			TFlecsComponentTraits<T>::PrePropertiesRegistration(ComponentHandle);

			switch (ComponentProperties.OnInstantiate)
			{
				case EFlecsOnInstantiate::Override:
					ComponentHandle.AddPair(flecs::OnInstantiate, flecs::Override);
					break;
				case EFlecsOnInstantiate::Inherit:
					ComponentHandle.AddPair(flecs::OnInstantiate, flecs::Inherit);
					break;
				case EFlecsOnInstantiate::DontInherit:
					ComponentHandle.AddPair(flecs::OnInstantiate, flecs::DontInherit);
					break;
				default:
					break;
			}

			switch (ComponentProperties.OnDelete)
			{
				case EFlecsOnDelete::Remove:
					ComponentHandle.AddPair(flecs::OnDelete, flecs::Remove);
					break;
				case EFlecsOnDelete::Delete:
					ComponentHandle.AddPair(flecs::OnDelete, flecs::Delete);
					break;
				case EFlecsOnDelete::Panic:
					ComponentHandle.AddPair(flecs::OnDelete, flecs::Panic);
					break;
				default:
					break;
			}

			switch (ComponentProperties.OnDeleteTarget)
			{
				case EFlecsOnDelete::Remove:
					ComponentHandle.AddPair(flecs::OnDeleteTarget, flecs::Remove);
					break;
				case EFlecsOnDelete::Delete:
					ComponentHandle.AddPair(flecs::OnDeleteTarget, flecs::Delete);
					break;
				case EFlecsOnDelete::Panic:
					ComponentHandle.AddPair(flecs::OnDeleteTarget, flecs::Panic);
					break;
				default:
					break;
			}

			if constexpr (TFlecsComponentTraits<T>::Trait)
			{
				ComponentHandle.Add(flecs::Trait);
			}

			if constexpr (TFlecsComponentTraits<T>::Sparse)
			{
				ComponentHandle.Add(flecs::Sparse);
			}

			if constexpr (TFlecsComponentTraits<T>::DontFragment)
			{
				ComponentHandle.Add(flecs::DontFragment);
			}

			if constexpr (TFlecsComponentTraits<T>::Relationship)
			{
				ComponentHandle.Add(flecs::Relationship);
			}

			if constexpr (TFlecsComponentTraits<T>::Target)
			{
				ComponentHandle.Add(flecs::Target);
			}

			if constexpr (TFlecsComponentTraits<T>::PairIsTag)
			{
				ComponentHandle.Add(flecs::PairIsTag);
			}

			if constexpr (TFlecsComponentTraits<T>::Reflexive)
			{
				ComponentHandle.Add(flecs::Reflexive);
			}

			if constexpr (TFlecsComponentTraits<T>::Acyclic)
			{
				ComponentHandle.Add(flecs::Acyclic);
			}

			if constexpr (TFlecsComponentTraits<T>::Traversable)
			{
				ComponentHandle.Add(flecs::Traversable);
			}

			if constexpr (TFlecsComponentTraits<T>::Transitive)
			{
				ComponentHandle.Add(flecs::Transitive);
			}

			if constexpr (TFlecsComponentTraits<T>::OneOf)
			{
				ComponentHandle.Add(flecs::OneOf);
			}

			if constexpr (TFlecsComponentTraits<T>::CanToggle)
			{
				ComponentHandle.Add(flecs::CanToggle);
			}

			if constexpr (TFlecsComponentTraits<T>::Inheritable)
			{
				ComponentHandle.Add(flecs::Inheritable);
			}

			if constexpr (TFlecsComponentTraits<T>::Final)
			{
				ComponentHandle.Add(flecs::Final);
			}

			if constexpr (TFlecsComponentTraits<T>::Replicate)
			{
				const FFlecsReplicationComponentDefinition ReplicationDefinition =
					UE::Flecs::Replication::MakeComponentDefinition<T>(ComponentHandle);

				
				const TValueOrError<void, FString> RegistrationResult 
					= UE::Flecs::FFlecsComponentRegistrationHooks
					::RegisterReplicatedComponent(InFlecsWorld, ReplicationDefinition);

				if UNLIKELY_IF(RegistrationResult.HasError())
				{
					UE_LOG(LogFlecsCore, Error,
						TEXT("Failed to register replicated component '%s': %s"),
						*ComponentProperties.Name, *RegistrationResult.GetError());
					return;
				}

				UE::Flecs::FFlecsComponentRegistrationHooks::MarkReplicatedComponent(ComponentHandle);
			}

			if constexpr (TFlecsComponentTraits<T>::WithAddReferencedObjects)
			{
				ComponentHandle.Add<FFlecsAddReferencedObjectsTrait>();
			}

			if constexpr (TFlecsComponentTraits<T>::Singleton)
			{
				ComponentHandle.Add(flecs::Singleton);

				if constexpr (TFlecsComponentTraits<T>::CustomSingletonValue)
				{
					if constexpr (!std::is_empty_v<T>)
					{
						if UNLIKELY_IF(!InFlecsWorld->Has<T>())
						{
							/*UE_LOGFMT(LogFlecsCore, Warning,
								"Singleton component {ComponentName} does not exist in the world, adding it now. This should have been added when flecs::Singleton was added to the component!",
								*ComponentProperties.Name);*/

							InFlecsWorld->Add<T>();
						}

						T& SingletonValue = InFlecsWorld->GetMut<T>();
						TFlecsComponentTraits<T>::SetSingletonDefaultValue(InFlecsWorld, SingletonValue);
					}
				}
			}

			const TSolidNotNull<const UFlecsWorldInterfaceObject*> WorldInterface =
				static_cast<const UFlecsWorldInterfaceObject*>(static_cast<const UFlecsWorld*>(InFlecsWorld));

			for (const FFlecsQueryGeneratorInput& WithType : ComponentProperties.WithTypes)
			{
				ComponentHandle.AddWith(WithType.GetFirstTermRef<false>(WorldInterface).Get<FFlecsId>());
			}

			for (const FFlecsQueryGeneratorInput& DependsOn : ComponentProperties.DependsOn)
			{
				ComponentHandle.AddPair(flecs::DependsOn,
					DependsOn.GetFirstTermRef<false>(WorldInterface).Get<FFlecsId>());
			}

			if (ComponentProperties.ChildOf.IsSet())
			{
				ComponentHandle.AddPair(flecs::ChildOf,
					ComponentProperties.ChildOf.GetValue().GetFirstTermRef<false>(WorldInterface).Get<FFlecsId>());
			}

			if (ComponentProperties.InheritsFrom.IsSet())
			{
				ComponentHandle.AddPair(flecs::IsA,
					ComponentProperties.InheritsFrom.GetValue().GetFirstTermRef<false>(WorldInterface).Get<FFlecsId>());
			}
			
			for (const FFlecsQueryGeneratorInput& CustomTrait : ComponentProperties.CustomTraits)
			{
				ComponentHandle.Add(CustomTrait.GetFirstTermRef<false>(WorldInterface).Get<FFlecsId>());
			}

			TFlecsComponentTraits<T>::PostRegister(ComponentHandle);
		};

		if constexpr (TFlecsComponentTraits<T>::AutoRegister)
		{
			Definition.RegistrationFunction = []
			(const TSolidNotNull<const UFlecsWorld*> InFlecsWorld, const FFlecsComponentPropertiesDefinition& ComponentProperties)
			{
				FFlecsId ScopeId = FFlecsId::Null();
				
				if (ComponentProperties.RegistrationScopeType != EUnrealFlecsRegistrationScopeType::None 
					&& !ComponentProperties.RegistrationScopeName.IsEmpty())
				{
					ScopeId = UE::Flecs::Registration::ResolveRegistrationScopeToId(InFlecsWorld,
						ComponentProperties.RegistrationScopeName, 
						ComponentProperties.RegistrationScopeType);
				}

				FFlecsComponentHandle RegisteredComponentHandle;

				if constexpr (Solid::IsScriptStruct<T>())
				{
					RegisteredComponentHandle = InFlecsWorld->RegisterComponentType<T>(ComponentProperties.bUseLowId,
						ComponentProperties.bRegisterMemberProperties);
				}
				else
				{
					RegisteredComponentHandle = InFlecsWorld->RegisterComponentType<T>(ComponentProperties.bUseLowId);
				}

				if LIKELY_IF(ScopeId.IsValid())
				{
					const FFlecsEntityView ScopeEntity = ScopeId.ToHandle<FFlecsEntityView>(InFlecsWorld->GetNativeFlecsWorld());
					RegisteredComponentHandle.AddPair(flecs::ChildOf, ScopeEntity);
				}
			};
		}
		else
		{
			Definition.RegistrationFunction = [](const TSolidNotNull<const UFlecsWorld*> InFlecsWorld, const FFlecsComponentPropertiesDefinition& ComponentProperties)
			{
				// Do nothing, user will manually call RegisterComponentType in this case
			};
		}

		return Definition;
	}

}; // struct FFlecsComponentProperties

DECLARE_MULTICAST_DELEGATE_OneParam(FOnComponentPropertiesRegistered, FFlecsComponentPropertiesDefinition);

namespace UE::Flecs::Private
{
	UNREALFLECS_API NO_DISCARD FCriticalSection& GetRegisteredComponentsMutex();

	UNREALFLECS_API NO_DISCARD TArray<FFlecsComponentPropertiesDefinition>& GetPendingRegisteredComponents();
	UNREALFLECS_API void AddRegisteredComponentProperties_Static(const FFlecsComponentPropertiesDefinition& InDefinition);

	UNREALFLECS_API EUnrealFlecsRegistrationScopeType FixupUnsetScopeType(
		const FFlecsComponentPropertiesDefinition& InDefinition,
		const FString& InModuleName,
		const FString& InPluginName);

	template <typename T>
	struct TFlecsComponentPropertiesRegistrar
	{
		UE_STATIC_ASSERT_WARN(std::is_enum<T>::value || (flecs::dont_fragment<T>::value == TFlecsComponentTraits<T>::DontFragment), 
			"Mismatch between flecs::dont_fragment trait and TFlecsComponentTraits::DontFragment, you should have a static constexpr bool DontFragment = true; in your Component's struct definition to match the flecs::dont_fragment trait.");
		
		UE_STATIC_ASSERT_WARN(std::is_enum<T>::value || (flecs::sparse<T>::value == TFlecsComponentTraits<T>::Sparse),
			"Mismatch between flecs::sparse trait and TFlecsComponentTraits::Sparse, you should have a static constexpr bool Sparse = true; in your Component's struct definition to match the flecs::sparse trait.");
		
		UE_STATIC_ASSERT_WARN(std::is_enum<T>::value
			|| (TFlecsComponentTraits<T>::OnInstantiate == EFlecsOnInstantiate::Override || flecs::on_instantiate_trait<T>::declared),
			"Mismatch between flecs::on_instantiate trait and TFlecsComponentTraits::OnInstantiate, you should have a static constexpr EFlecsOnInstantiate OnInstantiate = ...; in your Component's struct definition to match the flecs::on_instantiate trait.");
		
		
	public:
		TFlecsComponentPropertiesRegistrar(const FString& InModuleName = {}, const FString& InPluginName = {})
		{
			// @TODO: make safe
			FCoreDelegates::GetOnPostEngineInit().AddLambda([InModuleName, InPluginName]
			{
				FFlecsComponentPropertiesDefinition Definition = FFlecsComponentPropertiesDefinition::Make<T>();
				
				const bool bIsScopeTypeNone = Definition.RegistrationScopeType == EUnrealFlecsRegistrationScopeType::None;
				
				if (!bIsScopeTypeNone)
				{
					if (Definition.RegistrationScopeType == EUnrealFlecsRegistrationScopeType::Unset)
					{
						Definition.RegistrationScopeType = FixupUnsetScopeType(
							Definition, InModuleName, InPluginName);
					}

					if (Definition.RegistrationScopeType == EUnrealFlecsRegistrationScopeType::Module && !InModuleName.IsEmpty())
					{
						Definition.RegistrationScopeName = InModuleName;
					}
					else if (Definition.RegistrationScopeType == EUnrealFlecsRegistrationScopeType::Plugin && !InPluginName.IsEmpty())
					{
						Definition.RegistrationScopeName = InPluginName;
					}
				}

				if (!bIsScopeTypeNone && Definition.RegistrationScopeName.IsEmpty())
				{
					if (const UField* FieldObject = UE::Flecs::internal::GetMetaTypeIf<T>())
					{
						auto [OutScopeName, OutScopeType] = Registration::ResolveScopeTypeName(
								FieldObject, 
								Definition.RegistrationScopeType);
						
						Definition.RegistrationScopeName = OutScopeName;
						Definition.RegistrationScopeType = OutScopeType;
					}
				}

				AddRegisteredComponentProperties_Static(Definition);
			});
		}

	}; // struct TFlecsComponentPropertiesRegistrar

} // namespace UE::Flecs::Private

#define _INTERNAL_REGISTER_FLECS_COMPONENT_1(Name) \
	namespace \
	{ \
		static UE::Flecs::Private::TFlecsComponentPropertiesRegistrar<Name> FlecsComponentPropertiesRegistrarInstance_##Name { UE_MODULE_NAME }; \
	}

#define _INTERNAL_REGISTER_FLECS_COMPONENT_2(Name) \
	namespace \
	{ \
		static UE::Flecs::Private::TFlecsComponentPropertiesRegistrar<Name> FlecsComponentPropertiesRegistrarInstance_##Name { UE_MODULE_NAME, UE_PLUGIN_NAME }; \
	}

#ifdef UE_PLUGIN_NAME
#define _INTERNAL_FLECS_HAS_PLUGIN_NAME 1
#else // UE_PLUGIN_NAME
#define _INTERNAL_FLECS_HAS_PLUGIN_NAME 0
#endif // UE_PLUGIN_NAME

#define INTERNAL_REGISTER_FLECS_COMPONENT_IMPL(Name) \
	UE_IF(_INTERNAL_FLECS_HAS_PLUGIN_NAME, _INTERNAL_REGISTER_FLECS_COMPONENT_2, _INTERNAL_REGISTER_FLECS_COMPONENT_1)(Name)

// Use this
#define REGISTER_FLECS_COMPONENT(ComponentType, ...) \
	INTERNAL_REGISTER_FLECS_COMPONENT_IMPL(ComponentType)
