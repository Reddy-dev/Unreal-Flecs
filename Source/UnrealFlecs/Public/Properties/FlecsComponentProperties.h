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

#include "Logs/FlecsCategories.h"
#include "Worlds/FlecsWorld.h"
#include "Entities/FlecsComponentHandle.h"
#include "Components/FlecsAddReferencedObjectsTrait.h"
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

	namespace internal
	{
		template <typename T>
		NO_DISCARD FORCEINLINE UScriptStruct* GetScriptStructIf()
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
		
		template <typename T>
		NO_DISCARD FORCEINLINE UStruct* GetMetaTypeIf()
		{
			return nullptr;
		}
		
		template <Solid::TScriptStructConcept T>
		NO_DISCARD FORCEINLINE UStruct* GetMetaTypeIf()
		{
			return TBaseStructure<T>::Get();
		}
		
		template <Solid::TStaticClassConcept T>
		NO_DISCARD FORCEINLINE UStruct* GetMetaTypeIf()
		{
			return StaticClass<T>();
		}
		
		template <Solid::TStaticEnumConcept T>
		NO_DISCARD FORCEINLINE UStruct* GetMetaTypeIf()
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

template <typename T>
struct TFlecsComponentTraitsBase
{
public:
	static constexpr bool AutoRegister = true;
	
	using WithTypes = TTuple<>;
	using ChildOf = void;
	using DependsOn = TTuple<>;
	using InheritsFrom = void;
	
	static constexpr EFlecsOnInstantiate OnInstantiate = EFlecsOnInstantiate::Override;	
	static constexpr EFlecsOnDelete OnDelete = EFlecsOnDelete::Remove;
	static constexpr EFlecsOnDelete OnDeleteTarget = EFlecsOnDelete::Remove;
	
	static constexpr bool Singleton = false;
	static constexpr bool Trait = false;
	
	static constexpr bool DontFragment = false;
	static constexpr bool Sparse = DontFragment;
	
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
	static constexpr bool RegisterMemberProperties = true;
	static constexpr bool RegisterWithUnrealModule = std::is_void<ChildOf>::value;
	
	static FName GetOwningModule()
	{
		return NAME_None;
	}
	
	static const TArray<FString>& CustomTypeDependencies()
	{
		static const TArray<FString> EmptyArray;
		return EmptyArray;
	}
	
	// Only get called with Auto-Registration with Typed Components
	static void PrePropertiesRegistration(const FFlecsComponentHandle& ComponentHandle) {}
	static void PostRegister(const FFlecsComponentHandle& ComponentHandle) {}
	
}; // struct TFlecsComponentTraitsBase

template <typename T>
struct TFlecsComponentTraits : public TFlecsComponentTraitsBase<T>
{
	
}; // struct TFlecsComponentTraits

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
	
	// Only matters if the component is a UScriptStruct Type
	UPROPERTY()
	uint32 bRegisterMemberProperties : 1 = false;
	
	UPROPERTY()
	uint32 bRegisterWithModule : 1 = false;
	
	UPROPERTY()
	TArray<FFlecsQueryGeneratorInput> WithTypes;
	
	UPROPERTY()
	TOptional<FFlecsQueryGeneratorInput> ChildOf;
	
	UPROPERTY()
	TArray<FFlecsQueryGeneratorInput> DependsOn;
	
	UPROPERTY()
	TOptional<FFlecsQueryGeneratorInput> InheritsFrom;
	
	UPROPERTY()
	TArray<FString> CustomTypeDependencies;
	
	UPROPERTY()
	FName OwningModule;
	
	UE::Flecs::FFlecsComponentRegistrationFunction RegistrationFunction;
	UE::Flecs::FFlecsComponentPropertiesFunction PropertiesFunction;
	
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
			.bWithAddReferencedObjects = TFlecsComponentTraits<T>::WithAddReferencedObjects,
			.bRegisterMemberProperties = TFlecsComponentTraits<T>::RegisterMemberProperties,
			.bRegisterWithModule = TFlecsComponentTraits<T>::RegisterWithUnrealModule,
			.OwningModule = TFlecsComponentTraits<T>::GetOwningModule()
		};
		
		UE::Flecs::internal::ForEachInTuple<typename TFlecsComponentTraits<T>::WithTypes>([&Definition]<typename TWithType>(TWithType Type)
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
		
		UE::Flecs::internal::ForEachInTuple<typename TFlecsComponentTraits<T>::DependsOn>([&Definition]<typename TDependsOn>(TDependsOn Type)
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
			UE::Flecs::internal::ForEachInTuple<typename TFlecsComponentTraits<T>::ChildOf>([&Definition]<typename TChildOf>(TChildOf Type)
			{
				using FTypeValue = TChildOf;
				
				FFlecsQueryGeneratorInput Input;
				Input.bPair = false;
				Input.First.InitializeAs<FFlecsQueryGeneratorInputType_CPPType>();
				Input.First.GetMutable<FFlecsQueryGeneratorInputType_CPPType>().SymbolString = FString(nameof(FTypeValue).data());
				
				Definition.ChildOf = Input;
			});
		}
		
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
			
			if constexpr (TFlecsComponentTraits<T>::Singleton)
			{
				ComponentHandle.Add(flecs::Singleton);
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
				ComponentHandle.Add(flecs::Inherit);
			}
			
			if constexpr (TFlecsComponentTraits<T>::Final)
			{
				ComponentHandle.Add(flecs::Final);
			}
			
			if constexpr (TFlecsComponentTraits<T>::WithAddReferencedObjects)
			{
				ComponentHandle.Add<FFlecsAddReferencedObjectsTrait>();
			}
			
			const TSolidNotNull<const UFlecsWorldInterfaceObject*> WorldInterface =
				static_cast<const UFlecsWorldInterfaceObject*>(static_cast<const UFlecsWorld*>(InFlecsWorld));

			for (const FFlecsQueryGeneratorInput& WithType : ComponentProperties.WithTypes)
			{
				ComponentHandle.AddWith(WithType.GetFirstTermRef(WorldInterface).Get<FFlecsId>());
			}

			for (const FFlecsQueryGeneratorInput& DependsOn : ComponentProperties.DependsOn)
			{
				ComponentHandle.AddPair(flecs::DependsOn,
					DependsOn.GetFirstTermRef(WorldInterface).Get<FFlecsId>());
			}

			if (ComponentProperties.ChildOf.IsSet())
			{
				ComponentHandle.AddPair(flecs::ChildOf,
					ComponentProperties.ChildOf.GetValue().GetFirstTermRef(WorldInterface).Get<FFlecsId>());
			}

			if (ComponentProperties.InheritsFrom.IsSet())
			{
				ComponentHandle.AddPair(flecs::IsA,
					ComponentProperties.InheritsFrom.GetValue().GetFirstTermRef(WorldInterface).Get<FFlecsId>());
			}
			
			TFlecsComponentTraits<T>::PostRegister(ComponentHandle);
		};
		
		if constexpr (TFlecsComponentTraits<T>::AutoRegister)
		{
			Definition.RegistrationFunction = [](const TSolidNotNull<const UFlecsWorld*> InFlecsWorld, const FFlecsComponentPropertiesDefinition& ComponentProperties)
			{
				FFlecsEntityHandle OwningModuleEntity;
				if (ComponentProperties.bRegisterWithModule && !ComponentProperties.OwningModule.IsNone())
				{
					// @TODO: Query Shit instead of lookups
					OwningModuleEntity = InFlecsWorld->LookupEntity(ComponentProperties.OwningModule.ToString());
				}
				
				FFlecsComponentHandle RegisteredComponentHandle;
				
				if constexpr (Solid::IsScriptStruct<T>())
				{
					RegisteredComponentHandle = InFlecsWorld->RegisterComponentType<T>(ComponentProperties.bRegisterMemberProperties);
				}
				else
				{
					RegisteredComponentHandle = InFlecsWorld->RegisterComponentType<T>();
				}
				
				if (OwningModuleEntity.IsValid())
				{
					if (!OwningModuleEntity.Has(flecs::Module))
					{
						UE_LOGFMT(LogFlecsCore, Error, "Found entity is not registered as a module");
					}
					
					RegisteredComponentHandle.AddPair(flecs::ChildOf, OwningModuleEntity);
				}
			};
		}
		else
		{
			Definition.RegistrationFunction = [](const TSolidNotNull<const UFlecsWorld*> InFlecsWorld, const FFlecsComponentPropertiesDefinition& ComponentProperties)
			{
				// Do Nothing, user will manually call RegisterComponentType in this case
			};
		}
		
		return Definition;
	}
	
}; // struct FFlecsComponentProperties

DECLARE_MULTICAST_DELEGATE_OneParam(FOnComponentPropertiesRegistered, FFlecsComponentPropertiesDefinition);

namespace UE::Flecs::Private
{
	UNREALFLECS_API FCriticalSection& GetRegisteredComponentsMutex();
	
	UNREALFLECS_API TArray<FFlecsComponentPropertiesDefinition>& GetPendingRegisteredComponents();
	UNREALFLECS_API void AddRegisteredComponentProperties_Static(const FFlecsComponentPropertiesDefinition& InDefinition);
	
	template <typename T>
	struct TFlecsComponentPropertiesRegistrar
	{
	public:
		TFlecsComponentPropertiesRegistrar(const char* InModuleName = nullptr)
		{
			FCoreDelegates::OnPostEngineInit.AddLambda([InModuleName]()
			{
				FFlecsComponentPropertiesDefinition Definition = FFlecsComponentPropertiesDefinition::Make<T>();
				
				if (Definition.bRegisterWithModule && Definition.OwningModule.IsNone() && InModuleName != nullptr)
				{
					Definition.OwningModule = FName(InModuleName);
				}
				
				if (Definition.bRegisterWithModule && Definition.OwningModule.IsNone())
				{
					if (const UScriptStruct* ScriptStruct = UE::Flecs::internal::GetScriptStructIf<T>())
					{
						FString PackageName = ScriptStruct->GetOutermost()->GetName();
						FString ModuleStr;
						
						if (PackageName.Split(TEXT("/Script/"), nullptr, &ModuleStr) && !ModuleStr.IsEmpty())
						{
							Definition.OwningModule = FName(*ModuleStr);
						}
					}
				}
				
				AddRegisteredComponentProperties_Static(Definition);
			});
		}
	
	}; // struct TFlecsComponentPropertiesRegistrar
	
} // namespace UE::Flecs::Private

#define INTERNAL_REGISTER_FLECS_COMPONENT_IMPL(Name) \
	namespace \
	{ \
		static UE::Flecs::Private::TFlecsComponentPropertiesRegistrar<Name> FlecsComponentPropertiesRegistrarInstance_##Name { UE_MODULE_NAME }; \
	}

#define REGISTER_FLECS_COMPONENT(ComponentType, ...) \
	INTERNAL_REGISTER_FLECS_COMPONENT_IMPL(ComponentType)
	
	