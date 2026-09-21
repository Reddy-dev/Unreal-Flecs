// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "Types/SolidEnumSelector.h"

#include "EntityRecords/FlecsEntityRecord.h"
#include "FlecsCollectionTypes.h"
#include "FlecsCollectionDefinition.h"

#include "FlecsCollectionBuilder.generated.h"

struct FFlecsCollectionBuilder;

/**
 * @brief Fluent builder for the two slots of an FFlecsRecordPair.
 *
 * The builder accepts reflected script-struct values, entity ids, and
 * gameplay tags, then returns itself for chained configuration.
 */
struct UNREALFLECS_API FFlecsCollectionPairBuilder
{
	mutable FFlecsRecordPair Pair;
	
	/** Selects which pair slot supplies the value component. */
	FORCEINLINE const FFlecsCollectionPairBuilder& ValueIs(const EFlecsValuePairType InType) const
	{
		Pair.PairValueType = InType;
		return *this;
	}

	template <Solid::TScriptStructConcept T>
	/**
	 * @brief Sets the first pair slot to a script-struct type.
	 * @tparam T Non-utility script-struct type used as the first slot.
	 */
	FORCEINLINE const FFlecsCollectionPairBuilder& First() const
	{
		Pair.First.PairNodeType = EFlecsPairNodeType::ScriptStruct;
		Pair.First.PairScriptStruct = FInstancedStruct::Make<T>();
		
		return *this;
	}
	
	template <Solid::TScriptStructConcept T>
	/**
	 * @brief Sets the first pair slot to a script-struct value.
	 * @tparam T Non-utility script-struct type used as the first slot.
	 * @param ScriptStructValue Value stored in the first slot.
	 */
	FORCEINLINE const FFlecsCollectionPairBuilder& First(const T& ScriptStructValue) const
	{
		Pair.First.PairNodeType = EFlecsPairNodeType::ScriptStruct;
		Pair.First.PairScriptStruct = FInstancedStruct::Make<T>(ScriptStructValue);
		
		return *this;
	}

	/** Sets the first pair slot to an entity id. */
	FORCEINLINE const FFlecsCollectionPairBuilder& First(const FFlecsId& InEntityId) const
	{
		Pair.First.PairNodeType = EFlecsPairNodeType::EntityHandle;
		Pair.First.EntityHandle = InEntityId;
		
		return *this;
	}

	/** Sets the first pair slot to a gameplay tag. */
	FORCEINLINE const FFlecsCollectionPairBuilder& Add(const FGameplayTag& InTag) const
	{
		Pair.First.PairNodeType = EFlecsPairNodeType::FGameplayTag;
		Pair.First.GameplayTag = InTag;
		
		return *this;
	}
	
	template <Solid::TScriptStructConcept T>
	/**
	 * @brief Sets the second pair slot to a script-struct value.
	 * @tparam T Non-utility script-struct type used as the second slot.
	 * @param ScriptStructValue Value stored in the second slot.
	 */
	FORCEINLINE const FFlecsCollectionPairBuilder& Second(const T& ScriptStructValue) const
	{
		Pair.Second.PairNodeType = EFlecsPairNodeType::ScriptStruct;
		Pair.Second.PairScriptStruct = FInstancedStruct::Make<T>(ScriptStructValue);
		return *this;
	}

	/** Sets the second pair slot to an entity id. */
	FORCEINLINE const FFlecsCollectionPairBuilder& Second(const FFlecsId& InEntityId) const
	{
		Pair.Second.PairNodeType = EFlecsPairNodeType::EntityHandle;
		Pair.Second.EntityHandle = InEntityId;
		
		return *this;
	}
	
	/** Sets the second pair slot to a gameplay tag. */
	FORCEINLINE const FFlecsCollectionPairBuilder& Second(const FGameplayTag& InTag) const
	{
		Pair.Second.PairNodeType = EFlecsPairNodeType::FGameplayTag;
		Pair.Second.GameplayTag = InTag;
		
		return *this;
	}
	
}; // struct FFlecsPairBuilder

USTRUCT(BlueprintType)
/**
 * @brief Fluent builder for one Collection child entity.
 *
 * A sub-entity builder adds components and nested Collection references to
 * the child record created by BeginSubEntity(). Call EndSubEntity() to return
 * to the parent Collection builder.
 */
struct UNREALFLECS_API FFlecsSubEntityCollectionBuilder
{
	GENERATED_BODY()

public:
	/** Creates an unbound sub-entity builder. */
	FFlecsSubEntityCollectionBuilder() = default;
	/**
	 * @brief Creates a builder bound to a sub-entity record.
	 * @param InIdName Name assigned to the child entity.
	 * @param InRecord Record receiving the child components.
	 * @param InParentBuilder Parent Collection builder returned by EndSubEntity().
	 */
	FFlecsSubEntityCollectionBuilder(const FString& InIdName, FFlecsEntityRecord& InRecord, FFlecsCollectionBuilder* InParentBuilder)
		: IdName(InIdName)
		, Record(&InRecord)
		, ParentBuilder(InParentBuilder)
	{
	}

	/** Sets the name assigned to this child entity. */
	FORCEINLINE FFlecsSubEntityCollectionBuilder& Name(const FString& InName)
	{
		IdName = InName;
		return *this;
	}

	template <Solid::TScriptStructConcept T>
	/**
	 * @brief Adds a default-constructed script-struct component.
	 * @tparam T Script-struct component type.
	 * @return This sub-entity builder for chaining.
	 */
	FORCEINLINE FFlecsSubEntityCollectionBuilder& Add()
	{
		GetRecord().AddComponent<T>();
		return *this;
	}

	template <Solid::TScriptStructConcept T>
	/**
	 * @brief Adds a script-struct component value.
	 * @tparam T Script-struct component type.
	 * @param InComponent Component value to store in the child record.
	 * @return This sub-entity builder for chaining.
	 */
	FORCEINLINE FFlecsSubEntityCollectionBuilder& Add(const T& InComponent)
	{
		GetRecord().AddComponent<T>(InComponent);
		return *this;
	}

	/** Adds a reflected enum component to the child record. */
	FORCEINLINE FFlecsSubEntityCollectionBuilder& Add(const FSolidEnumSelector& InEnumSelector)
	{
		GetRecord().AddComponent(InEnumSelector);
		return *this;
	}

	/** Moves a reflected enum component into the child record. */
	FORCEINLINE FFlecsSubEntityCollectionBuilder& Add(FSolidEnumSelector&& InEnumSelector)
	{
		GetRecord().AddComponent(MoveTemp(InEnumSelector));
		return *this;
	}

	template <Solid::TStaticEnumConcept TEnum>
	/**
	 * @brief Adds a native enum component value to the child record.
	 * @tparam TEnum Static enum type.
	 * @param InEnumValue Enum value to store.
	 */
	FORCEINLINE FFlecsSubEntityCollectionBuilder& Add(const TEnum InEnumValue)
	{
		GetRecord().AddComponent<TEnum>(InEnumValue);
		return *this;
	}

	/** Adds an entity id component to the child record. */
	FORCEINLINE FFlecsSubEntityCollectionBuilder& Add(const FFlecsId InId)
	{
		GetRecord().AddComponent(InId);
		return *this;
	}

	/** Adds a gameplay tag component to the child record. */
	FORCEINLINE FFlecsSubEntityCollectionBuilder& Add(const FGameplayTag& InGameplayTag)
	{
		GetRecord().AddComponent(InGameplayTag);
		return *this;
	}

	/** Adds a pre-built pair component to the child record. */
	FORCEINLINE FFlecsSubEntityCollectionBuilder& Add(const FFlecsRecordPair& InPair)
	{
		GetRecord().AddComponent(InPair);
		return *this;
	}

	/** Adds a pre-built pair to the child record. */
	FORCEINLINE FFlecsSubEntityCollectionBuilder& AddPair(const FFlecsRecordPair& InPair)
	{
		GetRecord().AddComponent(InPair);
		return *this;
	}

	template <UE::Flecs::CNonStructUtilScriptStructType TFirst, UE::Flecs::CNonStructUtilScriptStructType TSecond>
	/**
	 * @brief Adds a pair made from two script-struct types.
	 * @tparam TFirst Type used for the first pair slot.
	 * @tparam TSecond Type used for the second pair slot.
	 */
	FORCEINLINE FFlecsSubEntityCollectionBuilder& AddPair()
	{
		FFlecsRecordPair Pair;
		Pair.First = FFlecsRecordPairSlot::Make<TFirst>();
		Pair.Second = FFlecsRecordPairSlot::Make<TSecond>();
		return AddPair(Pair);
	}

	/**
	 * @brief Composes a data-asset Collection into this child entity.
	 * @param InAsset Collection asset to reference.
	 * @param InParameters Optional Collection parameters.
	 * @return This sub-entity builder for chaining.
	 */
	FFlecsSubEntityCollectionBuilder& ReferenceCollection(const TSolidNotNull<UFlecsCollectionDataAsset*> InAsset,
	                                                      const FInstancedStruct& InParameters = FInstancedStruct());

	/**
	 * @brief Composes a registered Collection into this child entity.
	 * @param InId Identifier of the Collection to reference.
	 * @param InParameters Optional Collection parameters.
	 * @return This sub-entity builder for chaining.
	 */
	FFlecsSubEntityCollectionBuilder& ReferenceCollection(const FFlecsCollectionId& InId,
	                                                      const FInstancedStruct& InParameters = FInstancedStruct());

	/**
	 * @brief Composes a class-defined Collection into this child entity.
	 * @param InClass Class implementing IFlecsCollectionInterface.
	 * @param InParameters Optional Collection parameters.
	 * @return This sub-entity builder for chaining.
	 */
	FFlecsSubEntityCollectionBuilder& ReferenceCollection(const UClass* InClass,
	                                                      const FInstancedStruct& InParameters = FInstancedStruct());

	template <Solid::TStaticClassConcept T>
	/**
	 * @brief Composes the Collection represented by a typed interface class.
	 * @tparam T Collection class implementing IFlecsCollectionInterface.
	 * @param InParameters Optional Collection parameters.
	 * @return This sub-entity builder for chaining.
	 */
	FORCEINLINE FFlecsSubEntityCollectionBuilder& ReferenceCollection(const FInstancedStruct& InParameters = FInstancedStruct())
	{
		return ReferenceCollection(T::StaticClass(), InParameters);
	}

	/*
	FORCEINLINE FFlecsCollectionBuilder& MarkSlot() const
	{
		GetRecord().AddComponent<FFlecsCollectionSlotTag>();
		return *ParentBuilder;
	}*/

	/** Finishes this child definition and returns the parent builder. */
	FORCEINLINE FFlecsCollectionBuilder& EndSubEntity() const;

	/** Returns the configured child-entity name. */
	NO_DISCARD FORCEINLINE const FString& GetName() const
	{
		return IdName;
	}

	/** Returns the entity record receiving this child definition. */
	NO_DISCARD FORCEINLINE FFlecsEntityRecord& GetRecord() const
	{
		solid_cassume(Record);
		return *Record;
	}

	UPROPERTY()
	/** Name assigned to the child entity. */
	FString IdName;
	
	FFlecsEntityRecord* Record = nullptr;

	UPROPERTY()
	/** Index of the child record in the parent Collection definition. */
	int32 SlotIndex = INDEX_NONE;

	FFlecsCollectionBuilder* ParentBuilder = nullptr;
	
}; // struct FFlecsSubEntityCollectionBuilder

USTRUCT(BlueprintType)
/**
 * @brief Fluent builder for an Unreal-Flecs Collection definition.
 *
 * The builder writes an FFlecsCollectionDefinition. Add components and pairs
 * to the root record, create child entities with BeginSubEntity(), compose
 * other Collections with ReferenceCollection(), and optionally define typed
 * instantiation parameters.
 */
struct UNREALFLECS_API FFlecsCollectionBuilder
{
	GENERATED_BODY()

	/**
	 * @brief Creates a builder bound to a Collection definition.
	 * @param InDefinition Definition populated by this builder.
	 * @return A builder that writes to InDefinition.
	 */
	NO_DISCARD static FORCEINLINE FFlecsCollectionBuilder Create(FFlecsCollectionDefinition& InDefinition)
	{
		return FFlecsCollectionBuilder(InDefinition);
	}
	
public:
	/** Creates an unbound Collection builder. */
	FORCEINLINE FFlecsCollectionBuilder()
		: CollectionDefinition(nullptr)
	{
	}

	/** Creates a Collection builder bound to InDefinition. */
	FORCEINLINE explicit FFlecsCollectionBuilder(FFlecsCollectionDefinition& InDefinition)
		: CollectionDefinition(&InDefinition)
	{
	}

	template <Solid::TScriptStructConcept T>
	/**
	 * @brief Adds a default-constructed script-struct component to the root.
	 * @tparam T Script-struct component type.
	 * @return This Collection builder for chaining.
	 */
	FORCEINLINE const FFlecsCollectionBuilder& Add() const
	{
		solid_cassume(CollectionDefinition);
		
		GetCollectionDefinition().Record.AddComponent<T>();
		
		return *this;
	}
	
	template <Solid::TScriptStructConcept T>
	/**
	 * @brief Adds a script-struct component value to the root.
	 * @tparam T Script-struct component type.
	 * @param InComponent Component value to store.
	 * @return This Collection builder for chaining.
	 */
	FORCEINLINE const FFlecsCollectionBuilder& Add(const T& InComponent) const
	{
		solid_cassume(CollectionDefinition);
		
		GetCollectionDefinition().Record.AddComponent<T>(InComponent);
		
		return *this;
	}

	/** Adds a reflected enum component to the root record. */
	FORCEINLINE const FFlecsCollectionBuilder& Add(const FSolidEnumSelector& InEnumSelector) const
	{
		solid_cassume(CollectionDefinition);

		GetCollectionDefinition().Record.AddComponent(InEnumSelector);

		return *this;
	}

	/** Moves a reflected enum component into the root record. */
	FORCEINLINE const FFlecsCollectionBuilder& Add(FSolidEnumSelector&& InEnumSelector) const
	{
		solid_cassume(CollectionDefinition);

		GetCollectionDefinition().Record.AddComponent(MoveTemp(InEnumSelector));

		return *this;
	}

	template <Solid::TStaticEnumConcept TEnum>
	/**
	 * @brief Adds a native enum component value to the root record.
	 * @tparam TEnum Static enum type.
	 * @param InEnumValue Enum value to store.
	 */
	FORCEINLINE const FFlecsCollectionBuilder& Add(const TEnum InEnumValue) const
	{
		solid_cassume(CollectionDefinition);

		GetCollectionDefinition().Record.AddComponent<TEnum>(InEnumValue);

		return *this;
	}

	/** Adds an entity id component to the root record. */
	FORCEINLINE const FFlecsCollectionBuilder& Add(const FFlecsId InId) const
	{
		solid_cassume(CollectionDefinition);
		
		GetCollectionDefinition().Record.AddComponent(InId);
		
		return *this;
	}

	/** Adds a gameplay tag component to the root record. */
	FORCEINLINE const FFlecsCollectionBuilder& Add(const FGameplayTag& InGameplayTag) const
	{
		solid_cassume(CollectionDefinition);
		
		GetCollectionDefinition().Record.AddComponent(InGameplayTag);
		
		return *this;
	}

	/** Adds a pre-built pair component to the root record. */
	FORCEINLINE const FFlecsCollectionBuilder& Add(const FFlecsRecordPair& InPair) const
	{
		solid_cassume(CollectionDefinition);
		
		GetCollectionDefinition().Record.AddComponent(InPair);
		
		return *this;
	}

	/** Adds a pre-built pair to the root record. */
	FORCEINLINE const FFlecsCollectionBuilder& AddPair(const FFlecsRecordPair& InPair) const
	{
		solid_cassume(CollectionDefinition);
		
		GetCollectionDefinition().Record.AddComponent(InPair);
		
		return *this;
	}

	template <UE::Flecs::CNonStructUtilScriptStructType TFirst, UE::Flecs::CNonStructUtilScriptStructType TSecond>
	/**
	 * @brief Adds a pair made from two script-struct types.
	 * @tparam TFirst Type used for the first pair slot.
	 * @tparam TSecond Type used for the second pair slot.
	 */
	FORCEINLINE const FFlecsCollectionBuilder& AddPair() const
	{
		FFlecsRecordPair Pair;
		Pair.First = FFlecsRecordPairSlot::Make<TFirst>();
		Pair.Second = FFlecsRecordPairSlot::Make<TSecond>();
		return AddPair(Pair);
	}

	/**
	 * @brief Begins a child-entity definition.
	 * @param InName Optional name assigned to the child entity.
	 * @param InTemplateRecord Existing record copied into the child.
	 * @return A builder for the new child entity.
	 */
	FORCEINLINE FFlecsSubEntityCollectionBuilder BeginSubEntity(const FString& InName = FString(),
		const FFlecsEntityRecord& InTemplateRecord = FFlecsEntityRecord()) const
	{
		solid_cassume(CollectionDefinition);
		
		const int32 SubEntityIndex = GetCollectionDefinition().Record.AddSubEntity(InTemplateRecord, false);

		FFlecsEntityRecord& SubEntityRecordReference = 
			GetCollectionDefinition().Record.GetSubEntity(SubEntityIndex).Get<FFlecsEntityRecord>();

		FFlecsSubEntityCollectionBuilder SubEntityBuilder(InName,
			SubEntityRecordReference, const_cast<FFlecsCollectionBuilder*>(this));  // NOLINT(cppcoreguidelines-pro-type-const-cast)

		SubEntityBuilder.SlotIndex = SubEntityIndex;
		
		CollectionDefinition->SubEntityCollections.Add(SubEntityIndex, FFlecsSubEntityCollectionReferences{});
		return SubEntityBuilder;
	}

	/**
	 * @brief Composes a data-asset Collection into the root.
	 * @param InAsset Collection asset to reference.
	 * @param InParameters Optional Collection parameters.
	 * @return This Collection builder for chaining.
	 */
	FORCEINLINE const FFlecsCollectionBuilder& ReferenceCollection(const TSolidNotNull<UFlecsCollectionDataAsset*> InAsset,
		const FInstancedStruct& InParameters = FInstancedStruct()) const
	{
		FFlecsCollectionInstancedReference Ref;
		Ref.Collection.Asset = InAsset;
		Ref.Collection.Mode = EFlecsCollectionReferenceMode::Asset;
		Ref.Parameters = InParameters;
		
		GetCollectionDefinition().Collections.Add(Ref);
		
		return *this;
	}

	/**
	 * @brief Composes a registered Collection into the root.
	 * @param InId Identifier of the Collection to reference.
	 * @param InParameters Optional Collection parameters.
	 * @return This Collection builder for chaining.
	 */
	FORCEINLINE const FFlecsCollectionBuilder& ReferenceCollection(const FFlecsCollectionId& InId, const FInstancedStruct& InParameters = FInstancedStruct()) const
	{
		FFlecsCollectionInstancedReference Ref;
		Ref.Collection.Id = InId;
		Ref.Collection.Mode = EFlecsCollectionReferenceMode::Id;
		Ref.Parameters = InParameters;
		
		GetCollectionDefinition().Collections.Add(Ref);
		
		return *this;
	}

	template <Solid::TStaticClassConcept T>
	/**
	 * @brief Composes the Collection represented by a typed interface class.
	 * @tparam T Collection class implementing IFlecsCollectionInterface.
	 * @param InParameters Optional Collection parameters.
	 * @return This Collection builder for chaining.
	 */
	FORCEINLINE const FFlecsCollectionBuilder& ReferenceCollection(const FInstancedStruct& InParameters = FInstancedStruct()) const
	{
		FFlecsCollectionInstancedReference Ref;
		Ref.Collection.Mode = EFlecsCollectionReferenceMode::UClass;
		Ref.Collection.Class = T::StaticClass();
		Ref.Parameters = InParameters;

		GetCollectionDefinition().Collections.Add(Ref);

		return *this;
	}
	
	/** Sets the name used when the definition is registered. */
	FORCEINLINE const FFlecsCollectionBuilder& Name(const FString& InName) const
	{
		IdName = InName;
		
		return *this;
	}

	/**
	 * @brief Defines parameter type, default value, and application callback.
	 * @param InParameters Default parameters stored on the Collection prefab.
	 * @param InApplyFunction Callback invoked for each Collection instance.
	 * @return This Collection builder for chaining.
	 */
	FORCEINLINE const FFlecsCollectionBuilder& Parameters(const FInstancedStruct& InParameters,
		const FFlecsCollectionParametersComponent::FApplyParametersFunction& InApplyFunction) const
	{
		solid_cassume(CollectionDefinition);

		FFlecsCollectionParametersComponent ParametersComponent;
		ParametersComponent.ParameterType = InParameters;
		ParametersComponent.ApplyParametersFunction = InApplyFunction;
		
		GetCollectionDefinition().Record.AddComponent<FFlecsCollectionParametersComponent>(MoveTemp(ParametersComponent));
		
		return *this;
	}

	template <Solid::TScriptStructConcept T, typename TApplyFunction>
	requires (!std::is_same<T, FInstancedStruct>::value)
	/**
	 * @brief Defines typed parameters and a typed application callback.
	 * @tparam T Script-struct type of the Collection parameters.
	 * @tparam TApplyFunction Callable accepting the target entity and T.
	 * @param InParameters Default parameters stored on the Collection prefab.
	 * @param InApplyFunction Callback invoked for each Collection instance.
	 * @return This Collection builder for chaining.
	 */
	FORCEINLINE const FFlecsCollectionBuilder& Parameters(const T& InParameters, TApplyFunction&& InApplyFunction) const
	{
		solid_cassume(CollectionDefinition);

		FFlecsCollectionParametersComponent ParametersComponent;
		ParametersComponent.ParameterType = FInstancedStruct::Make<T>(InParameters);
		ParametersComponent.ApplyParametersFunction = [InApplyFunction = SOLID_FWD(InApplyFunction)]
			(FFlecsEntityHandle TargetEntity, const FInstancedStruct& Parameters)
			{
				InApplyFunction(TargetEntity, Parameters.Get<T>());
			};
		
		GetCollectionDefinition().Record.AddComponent<FFlecsCollectionParametersComponent>(MoveTemp(ParametersComponent));
		
		return *this;
	}

	// @TODO: add other relationship overloads?
	
	/*void MarkSlot() const
	{
		solid_cassume(CollectionDefinition);
		
		GetCollectionDefinition().Record.AddComponent<FFlecsCollectionSlotTag>();
	}*/

	/** Returns the definition populated by this builder. */
	NO_DISCARD FORCEINLINE FFlecsCollectionDefinition& GetCollectionDefinition() const
	{
		solid_cassume(CollectionDefinition);
		
		return *CollectionDefinition;
	}

	UPROPERTY()
	/** Name assigned by Name() and used by registration helpers. */
	mutable FString IdName;
	
	mutable FFlecsCollectionDefinition* CollectionDefinition;
	
}; // struct FFlecsCollectionBuilder
