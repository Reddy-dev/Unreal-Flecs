// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "StructUtils/InstancedStruct.h"

#include "Concepts/SolidConcepts.h"
#include "Types/SolidEnumSelector.h"

#include "Entities/FlecsEntityHandle.h"

#include "FlecsEntityRecord.generated.h"

struct FFlecsEntityRecord;

UENUM(BlueprintType)
/**
 * @brief Identifies the kind of component data stored by an entity record.
 *
 * The node may describe a reflected struct, entity identifier, gameplay tag,
 * pair, or script enum.
 */
enum class EFlecsComponentNodeType : uint8
{
	ScriptStruct = 0,
	EntityHandle = 1,
	FGameplayTag = 2,
	Pair = 3, /* All Pairs if both are component types then the first type is assumed as value */
	ScriptEnum = 4,
}; // enum class EFlecsComponentNodeType

UENUM(BlueprintType)
/**
 * @brief Identifies the value representation used by one record-pair slot.
 */
enum class EFlecsPairNodeType : uint8
{
	ScriptStruct = 0,
	EntityHandle = 1,
	FGameplayTag = 2,
	//ScriptEnum = 3,
}; // enum class EFlecsPairNodeType

namespace UE::Flecs
{
	template <typename T>
	/**
	 * @brief Constrains values that can be stored in a record-pair slot.
	 * @tparam T Candidate slot value type.
	 */
	concept CRecordPairSlotType = std::is_convertible<T, FFlecsId>::value
		|| std::is_convertible<T, FGameplayTag>::value
		|| std::is_convertible<T, FInstancedStruct>::value;
	
	template <typename T>
	/**
	 * @brief Constrains script-struct types that are not StructUtils types.
	 * @tparam T Candidate script-struct type.
	 */
	concept CNonStructUtilScriptStructType = Solid::TScriptStructConcept<T> && !Solid::TStructUtilsTypeConcept<T>;
		
	namespace Entity::Records
	{
		template <typename TParentBuilder, typename TFragmentType>
		/**
		 * @brief Detects whether a fragment provides its own scoped builder.
		 * @tparam TParentBuilder Parent entity-record builder type.
		 * @tparam TFragmentType Fragment type being configured.
		 */
		concept CHasCustomFragmentBuilder = requires(TParentBuilder& Parent, TFragmentType& InFragment)
		{
			typename TFragmentType::FBuilder;
			{ typename TFragmentType::FBuilder(Parent, InFragment) };
			
		}; // concept CHasCustomFragmentBuilder
		
	} // namespace Entity::Records
	
} // namespace UE::Flecs

USTRUCT(BlueprintType)
/**
 * @brief Describes one side of an entity-record pair.
 *
 * A slot can contain a reflected struct value or type, an entity identifier,
 * or a gameplay tag.
 */
struct UNREALFLECS_API FFlecsRecordPairSlot
{
	GENERATED_BODY()

	/**
	 * @brief Creates a script-struct slot containing the supplied value.
	 * @param InStruct Reflected struct value to copy into the slot.
	 * @return A pair slot containing a script-struct value.
	 */
	static NO_DISCARD FFlecsRecordPairSlot Make(const FInstancedStruct& InStruct)
	{
		FFlecsRecordPairSlot OutSlot;
		OutSlot.PairNodeType = EFlecsPairNodeType::ScriptStruct;
		OutSlot.PairScriptStruct = InStruct;
		return OutSlot;
	}

	/**
	 * @brief Creates a script-struct slot initialized from a type.
	 * @param InStructType Script-struct type to initialize in the slot.
	 * @return A pair slot containing an initialized script-struct value.
	 */
	static NO_DISCARD FFlecsRecordPairSlot Make(const TSolidNotNull<const UScriptStruct*> InStructType)
	{
		FFlecsRecordPairSlot OutSlot;
		OutSlot.PairNodeType = EFlecsPairNodeType::ScriptStruct;

		FInstancedStruct NewInstancedStruct;
		NewInstancedStruct.InitializeAs(InStructType);
		
		OutSlot.PairScriptStruct = SOLID_MOV(NewInstancedStruct);
		return OutSlot;
	}

	template <UE::Flecs::CNonStructUtilScriptStructType T>
	/**
	 * @brief Creates a default-initialized script-struct slot.
	 * @tparam T Script-struct type to initialize.
	 * @return A pair slot containing a default value of type T.
	 */
	static NO_DISCARD FFlecsRecordPairSlot Make()
	{
		FFlecsRecordPairSlot OutSlot;
		OutSlot.PairNodeType = EFlecsPairNodeType::ScriptStruct;
		OutSlot.PairScriptStruct = FInstancedStruct::Make<T>();
		return OutSlot;
	}

	template <UE::Flecs::CNonStructUtilScriptStructType T>
	/**
	 * @brief Creates a script-struct slot containing a typed value.
	 * @tparam T Script-struct type stored by the slot.
	 * @param InValue Value to copy into the slot.
	 * @return A pair slot containing the supplied value.
	 */
	static NO_DISCARD FFlecsRecordPairSlot Make(const T& InValue)
	{
		FFlecsRecordPairSlot OutSlot;
		OutSlot.PairNodeType = EFlecsPairNodeType::ScriptStruct;
		OutSlot.PairScriptStruct = FInstancedStruct::Make<T>(InValue);
		return OutSlot;
	}
	
	/**
	 * @brief Creates an entity-identifier slot.
	 * @param InEntityHandle Entity identifier stored by the slot.
	 * @return A pair slot containing the entity identifier.
	 */
	static NO_DISCARD FFlecsRecordPairSlot Make(const FFlecsId InEntityHandle)
	{
		FFlecsRecordPairSlot OutSlot;
		OutSlot.PairNodeType = EFlecsPairNodeType::EntityHandle;
		OutSlot.EntityHandle = InEntityHandle;
		return OutSlot;
	}

	/**
	 * @brief Creates a gameplay-tag slot.
	 * @param InGameplayTag Gameplay tag stored by the slot.
	 * @return A pair slot containing the gameplay tag.
	 */
	static NO_DISCARD FFlecsRecordPairSlot Make(const FGameplayTag& InGameplayTag)
	{
		FFlecsRecordPairSlot OutSlot;
		OutSlot.PairNodeType = EFlecsPairNodeType::FGameplayTag;
		OutSlot.GameplayTag = InGameplayTag;
		return OutSlot;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component Tree")
	/** Identifies which representation is active in this slot. */
	EFlecsPairNodeType PairNodeType = EFlecsPairNodeType::ScriptStruct;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component Tree",
		meta = (EditCondition = "PairNodeType == EFlecsPairNodeType::ScriptStruct", EditConditionHides))
	/** Reflected struct value or type stored when PairNodeType is ScriptStruct. */
	FInstancedStruct PairScriptStruct;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component Tree",
	//	meta = (EditCondition = "PairNodeType == EFlecsPairNodeType::ScriptEnum", EditConditionHides))
	//FSolidEnumSelector PairScriptEnum;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component Tree",
		meta = (EditCondition = "PairNodeType == EFlecsPairNodeType::EntityHandle", EditConditionHides))
	/** Entity identifier stored when PairNodeType is EntityHandle. */
	FFlecsId EntityHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component Tree",
		meta = (EditCondition = "PairNodeType == EFlecsPairNodeType::FGameplayTag", EditConditionHides))
	/** Gameplay tag stored when PairNodeType is FGameplayTag. */
	FGameplayTag GameplayTag;

	/**
	 * @brief Compares the active value of this pair slot with another slot.
	 * @param Other Slot to compare with.
	 * @return True when both slots have the same representation and value.
	 */
	NO_DISCARD FORCEINLINE bool UEOpEquals(const FFlecsRecordPairSlot& Other) const
	{
		switch (PairNodeType)
		{
		case EFlecsPairNodeType::ScriptStruct:
			{
				return PairNodeType == Other.PairNodeType && PairScriptStruct == Other.PairScriptStruct;
			}
		case EFlecsPairNodeType::EntityHandle:
			{
				return PairNodeType == Other.PairNodeType && EntityHandle == Other.EntityHandle;
			}
		case EFlecsPairNodeType::FGameplayTag:
			{
				return PairNodeType == Other.PairNodeType && GameplayTag == Other.GameplayTag;
			}
		}

		return false;
	}
	
}; // struct FFlecsPairSlot

UENUM(BlueprintType)
/**
 * @brief Identifies which side of a pair supplies its value when a slot stores
 * an instanced struct.
 */
enum class EFlecsValuePairType : uint8
{
	None = 0,
	First = 1,
	Second = 2,
}; // enum class EFlecsValuePairType

USTRUCT(BlueprintType)
/**
 * @brief Describes a Flecs pair component for an entity record.
 *
 * Each side can be a script-struct value or type, an entity identifier, or a
 * gameplay tag. The pair is materialized when the record is applied.
 */
struct UNREALFLECS_API FFlecsRecordPair
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component Tree")
	/** First element of the pair. */
	FFlecsRecordPairSlot First;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component Tree")
	/** Second element of the pair. */
	FFlecsRecordPairSlot Second;

	// @TODO: make an edit condition for this?
	/**
	 * @brief Only needed if either First or Second are Instanced Structs
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component Tree",
		meta = (EditCondition = "First.PairNodeType == EFlecsPairNodeType::ScriptStruct || Second.PairNodeType == EFlecsPairNodeType::ScriptStruct",
		EditConditionHides))
	EFlecsValuePairType PairValueType = EFlecsValuePairType::None;

	/**
	 * @brief Compares both pair slots with another record pair.
	 * @param Other Pair to compare with.
	 * @return True when the pair slots are equal.
	 */
	NO_DISCARD FORCEINLINE bool UEOpEquals(const FFlecsRecordPair& Other) const
	{
		return First == Other.First && Second == Other.Second;
	}
	
	/**
	 * @brief Adds this pair as a component to an entity.
	 * @param InEntityHandle Entity that receives the pair.
	 */
	void AddToEntity(const FFlecsEntityHandle& InEntityHandle) const;

private:
	
	template <UE::Flecs::CRecordPairSlotType T>
	NO_DISCARD FFlecsRecordPairSlot MakeSlot(T&& InValue) const
	{
		FFlecsRecordPairSlot OutSlot;
		
		if constexpr (std::is_convertible<T, FInstancedStruct>::value)
		{
			OutSlot.PairNodeType = EFlecsPairNodeType::ScriptStruct;
			OutSlot.PairScriptStruct = MoveTemp(InValue);
		}
		else if constexpr (std::is_convertible<T, FFlecsId>::value)
		{
			OutSlot.PairNodeType = EFlecsPairNodeType::EntityHandle;
			OutSlot.EntityHandle = MoveTemp(InValue);
		}
		else if constexpr (std::is_convertible<T, FGameplayTag>::value)
		{
			OutSlot.PairNodeType = EFlecsPairNodeType::FGameplayTag;
			OutSlot.GameplayTag = MoveTemp(InValue);
		}
		else
		{
			static_assert(false, "Type T is not a valid Record Pair Slot Type");
		}
		
		return OutSlot;
	}

}; // struct FFlecsPair

USTRUCT(BlueprintType)
/**
 * @brief Describes one component entry stored in an entity record.
 *
 * The active representation is selected by NodeType and may contain a
 * reflected struct, script enum, entity identifier, gameplay tag, or pair.
 */
struct UNREALFLECS_API FFlecsComponentTypeInfo final
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component Tree")
	/** Identifies which component representation is active. */
	EFlecsComponentNodeType NodeType = EFlecsComponentNodeType::ScriptStruct;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component Tree",
		meta = (EditCondition = "NodeType == EFlecsComponentNodeType::ScriptStruct", EditConditionHides))
	/** Reflected component value or type stored for a ScriptStruct node. */
	FInstancedStruct ScriptStruct;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component Tree",
		meta = (EditCondition = "NodeType == EFlecsComponentNodeType::ScriptEnum", EditConditionHides))
	/** Script enum value stored for a ScriptEnum node. */
	FSolidEnumSelector ScriptEnum;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component Tree",
		meta = (EditCondition = "NodeType == EFlecsComponentNodeType::EntityHandle", EditConditionHides))
	/** Entity identifier stored for an EntityHandle node. */
	FFlecsId EntityHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component Tree",
		meta = (EditCondition = "NodeType == EFlecsComponentNodeType::FGameplayTag", EditConditionHides))
	/** Gameplay tag stored for an FGameplayTag node. */
	FGameplayTag GameplayTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component Tree",
		meta = (EditCondition = "NodeType == EFlecsComponentNodeType::Pair", EditConditionHides))
	/** Pair descriptor stored for a Pair node. */
	FFlecsRecordPair Pair;

	/**
	 * @brief Compares the active component representation with another descriptor.
	 * @param Other Descriptor to compare with.
	 * @return True when both descriptors represent the same component value.
	 */
	NO_DISCARD FORCEINLINE bool UEOpEquals(const FFlecsComponentTypeInfo& Other) const
	{
		switch (NodeType)
		{
			case EFlecsComponentNodeType::ScriptStruct:
				{
					return NodeType == Other.NodeType && ScriptStruct == Other.ScriptStruct && Pair == Other.Pair;
				}
			case EFlecsComponentNodeType::ScriptEnum:
				{
					return NodeType == Other.NodeType
						&& ScriptEnum.Class == Other.ScriptEnum.Class && ScriptEnum.Value == Other.ScriptEnum.Value;
				}
			case EFlecsComponentNodeType::EntityHandle:
				{
					return NodeType == Other.NodeType && EntityHandle == Other.EntityHandle;
				}
			case EFlecsComponentNodeType::FGameplayTag:
				{
					return NodeType == Other.NodeType && GameplayTag == Other.GameplayTag;
				}
			case EFlecsComponentNodeType::Pair:
				{
					return NodeType == Other.NodeType && Pair == Other.Pair;
				}
		}

		return false;
	}
	
}; // struct FFlecsComponentTypeInfo

USTRUCT(BlueprintInternalUseOnly)
/**
 * @brief Extension point for record data that participates in application.
 *
 * Derived fragments can run custom work immediately before and after the
 * record's components and child entities are applied.
 */
struct UNREALFLECS_API FFlecsEntityRecordFragment
{
	GENERATED_BODY()

public:
	/** Constructs an empty record fragment. */
	FFlecsEntityRecordFragment() = default;
	
	/** Virtual destructor for derived record fragments. */
	virtual ~FFlecsEntityRecordFragment()
	{
	}

	/**
	 * @brief Runs before the record contents are applied to an entity.
	 * @param InFlecsWorld World interface used for the application.
	 * @param InEntityHandle Entity receiving the record.
	 */
	virtual void PreApplyRecordToEntity(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InFlecsWorld, const FFlecsEntityHandle& InEntityHandle) const {}
	/**
	 * @brief Runs after the record contents are applied to an entity.
	 * @param InFlecsWorld World interface used for the application.
	 * @param InEntityHandle Entity receiving the record.
	 */
	virtual void PostApplyRecordToEntity(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InFlecsWorld, const FFlecsEntityHandle& InEntityHandle) const {}
	
}; // struct FFlecsEntityRecordFragment

// @TODO: maybe align on a 64-byte boundary?
// @TODO: make custom UI for FFlecsEntityRecord and make custom meta tags.
// @TODO: add data validation.

USTRUCT(BlueprintType)
/**
 * @brief Stores a child entity record and its parent-child relationship policy.
 */
struct UNREALFLECS_API FFlecsSubEntityRecord
{
	GENERATED_BODY()
	
public:
	/** Constructs an empty child record. */
	FORCEINLINE FFlecsSubEntityRecord() = default;
	
	/**
	 * @brief Constructs a child record by copying an instanced entity record.
	 * @param InRecord Child entity record to copy.
	 * @param bInDontFragmentParentChildRelationship Whether to avoid fragmenting the parent-child relationship when applied.
	 */
	FORCEINLINE explicit FFlecsSubEntityRecord(const TInstancedStruct<FFlecsEntityRecord>& InRecord, 
		const bool bInDontFragmentParentChildRelationship = true)
		: bDontFragmentParentChildRelationship(bInDontFragmentParentChildRelationship)
		, Record(InRecord)
	{
	}
	
	/**
	 * @brief Constructs a child record by moving an instanced entity record.
	 * @param InRecord Child entity record to move.
	 * @param bInDontFragmentParentChildRelationship Whether to avoid fragmenting the parent-child relationship when applied.
	 */
	FORCEINLINE explicit FFlecsSubEntityRecord(TInstancedStruct<FFlecsEntityRecord>&& InRecord, 
		const bool bInDontFragmentParentChildRelationship = true)
		: bDontFragmentParentChildRelationship(bInDontFragmentParentChildRelationship)
		, Record(MoveTemp(InRecord))
	{
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity Record")
	/** Controls fragmentation of the parent-child relationship when applied. */
	bool bDontFragmentParentChildRelationship = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity Record")
	/** Entity record used to construct the child entity. */
	TInstancedStruct<FFlecsEntityRecord> Record;
	
	/**
	 * @brief Compares the child record and relationship policy with another entry.
	 * @param Other Child record to compare with.
	 * @return True when both entries are equal.
	 */
	NO_DISCARD FORCEINLINE bool UEOpEquals(const FFlecsSubEntityRecord& Other) const
	{
		return bDontFragmentParentChildRelationship == Other.bDontFragmentParentChildRelationship
			&& Record == Other.Record;
	}
	
}; // struct FFlecsSubEntityRecord

/**
 * @brief Describes a generic entity and its child-entity hierarchy.
 *
 * The record stores reflected component descriptors, nested child records, and
 * optional fragments that can customize application. It can be materialized
 * onto an entity with ApplyRecordToEntity.
 *
 * @see FFlecsEntityRecord::Builder
 * @see FFlecsEntityRecord::ApplyRecordToEntity
 */
USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsEntityRecord final
{
	GENERATED_BODY()
	
public:

	/** Constructs an empty entity record. */
	FORCEINLINE FFlecsEntityRecord() = default;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity Record")
	/** Component descriptors to apply to the entity. */
	TArray<FFlecsComponentTypeInfo> Components;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity Record")
	/** Child entity records to construct below the entity. */
	TArray<FFlecsSubEntityRecord> SubEntities;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity Record", meta = (ExcludeBaseStruct, NoElementDuplicate))
	/** Optional extension fragments invoked during record application. */
	TArray<TInstancedStruct<FFlecsEntityRecordFragment>> Fragments;
	
	/**
	 * @brief Fluent builder for constructing an entity record.
	 *
	 * The builder forwards component, fragment, and child-record additions to its
	 * parent record and supports scoped fragment and sub-entity construction.
	 */
	struct FBuilder
	{
	public:
		/**
		 * @brief Creates a builder for an existing record.
		 * @param InRecord Record that receives the builder's additions.
		 */
		FORCEINLINE explicit FBuilder(FFlecsEntityRecord& InRecord)
			: EntityRecord(InRecord)
		{
		}
		
		/**
		 * @brief Returns the mutable record being built.
		 * @return The record passed to the builder.
		 */
		FORCEINLINE FFlecsEntityRecord& Build()
		{
			return EntityRecord;
		}
		
		/**
		 * @brief Returns the record being built as a const reference.
		 * @return The record passed to the builder.
		 */
		FORCEINLINE const FFlecsEntityRecord& Build() const
		{
			return EntityRecord;
		}
		
		template <UE::Flecs::CNonStructUtilScriptStructType T>
		/**
		 * @brief Adds a default-initialized script-struct component.
		 * @tparam T Script-struct component type.
		 * @return This builder.
		 */
		FORCEINLINE FBuilder& Component()
		{
			EntityRecord.AddComponent<T>();
			return *this;
		}

		template <UE::Flecs::CNonStructUtilScriptStructType T>
		/**
		 * @brief Adds a copied typed script-struct component.
		 * @tparam T Script-struct component type.
		 * @param InValue Component value to copy.
		 * @return This builder.
		 */
		FORCEINLINE FBuilder& Component(const T& InValue)
		{
			EntityRecord.AddComponent<T>(InValue);
			return *this;
		}

		template <UE::Flecs::CNonStructUtilScriptStructType T>
		/**
		 * @brief Adds a moved typed script-struct component.
		 * @tparam T Script-struct component type.
		 * @param InValue Component value to move.
		 * @return This builder.
		 */
		FORCEINLINE FBuilder& Component(T&& InValue)
		{
			EntityRecord.AddComponent<T>(MoveTemp(InValue));
			return *this;
		}
		
		/**
		 * @brief Adds an entity identifier as a component.
		 * @param InEntityHandle Entity identifier to add.
		 * @return This builder.
		 */
		FORCEINLINE FBuilder& Component(const FFlecsId InEntityHandle)
		{
			EntityRecord.AddComponent(InEntityHandle);
			return *this;
		}
		
		/**
		 * @brief Adds a reflected script-struct type as a component.
		 * @param InStructType Script-struct type to add.
		 * @return This builder.
		 */
		FORCEINLINE FBuilder& Component(const TSolidNotNull<const UScriptStruct*> InStructType)
		{
			EntityRecord.AddComponent(InStructType);
			return *this;
		}
		
		/**
		 * @brief Adds a copied instanced-struct component.
		 * @param InStruct Struct value to copy.
		 * @return This builder.
		 */
		FORCEINLINE FBuilder& Component(const FInstancedStruct& InStruct)
		{
			EntityRecord.AddComponent(InStruct);
			return *this;
		}
		
		/**
		 * @brief Adds a moved instanced-struct component.
		 * @param InStruct Struct value to move.
		 * @return This builder.
		 */
		FORCEINLINE FBuilder& Component(FInstancedStruct&& InStruct)
		{
			EntityRecord.AddComponent(MoveTemp(InStruct));
			return *this;
		}

		/**
		 * @brief Adds a gameplay tag as a component.
		 * @param InGameplayTag Gameplay tag to add.
		 * @return This builder.
		 */
		FORCEINLINE FBuilder& GameplayTag(const FGameplayTag& InGameplayTag)
		{
			EntityRecord.AddComponent(InGameplayTag);
			return *this;
		}

		/**
		 * @brief Adds a moved gameplay tag as a component.
		 * @param InGameplayTag Gameplay tag to move.
		 * @return This builder.
		 */
		FORCEINLINE FBuilder& GameplayTag(FGameplayTag&& InGameplayTag)
		{
			EntityRecord.AddComponent(MoveTemp(InGameplayTag));
			return *this;
		}

		/**
		 * @brief Adds a reflected script-enum value as a component.
		 * @param InScriptEnum Enum selector to copy.
		 * @return This builder.
		 */
		FORCEINLINE FBuilder& Enum(const FSolidEnumSelector& InScriptEnum)
		{
			EntityRecord.AddComponent(InScriptEnum);
			return *this;
		}

		/**
		 * @brief Adds a moved script-enum value as a component.
		 * @param InScriptEnum Enum selector to move.
		 * @return This builder.
		 */
		FORCEINLINE FBuilder& Enum(FSolidEnumSelector&& InScriptEnum)
		{
			EntityRecord.AddComponent(MoveTemp(InScriptEnum));
			return *this;
		}
		
		template <Solid::TStaticEnumConcept TEnum>
		/**
		 * @brief Adds a native enum value as a component.
		 * @tparam TEnum Registered native enum type.
		 * @param InEnumValue Enum value to add.
		 * @return This builder.
		 */
		FORCEINLINE FBuilder& Enum(const TEnum InEnumValue)
		{
			EntityRecord.AddComponent<TEnum>(InEnumValue);
			return *this;
		}

		/**
		 * @brief Adds a copied record pair as a component.
		 * @param InPair Pair to copy.
		 * @return This builder.
		 */
		FORCEINLINE FBuilder& Pair(const FFlecsRecordPair& InPair)
		{
			EntityRecord.AddComponent(InPair);
			return *this;
		}

		/**
		 * @brief Adds a moved record pair as a component.
		 * @param InPair Pair to move.
		 * @return This builder.
		 */
		FORCEINLINE FBuilder& Pair(FFlecsRecordPair&& InPair)
		{
			EntityRecord.AddComponent(MoveTemp(InPair));
			return *this;
		}
		
		template <UE::Flecs::CNonStructUtilScriptStructType TFragmentType, typename... TArgs>
		requires (std::is_base_of_v<FFlecsEntityRecordFragment, TFragmentType>)
		/**
		 * @brief Adds a typed record fragment constructed from arguments.
		 * @tparam TFragmentType Fragment type derived from FFlecsEntityRecordFragment.
		 * @tparam TArgs Constructor argument types.
		 * @param InArgs Arguments forwarded to the fragment constructor.
		 * @return This builder.
		 */
		FORCEINLINE FBuilder& Fragment(TArgs&&... InArgs)
		{
			EntityRecord.AddFragment<TFragmentType>(Forward<TArgs>(InArgs)...);
			
			return *this;
		}
		
		/**
		 * @brief Adds an instanced record fragment.
		 * @param InFragment Fragment instance to copy.
		 * @return This builder.
		 */
		FORCEINLINE FBuilder& Fragment(const TInstancedStruct<FFlecsEntityRecordFragment>& InFragment)
		{
			EntityRecord.AddFragment(InFragment);
			return *this;
		}
		
		template <UE::Flecs::CNonStructUtilScriptStructType TFragmentType>
		requires (std::is_base_of_v<FFlecsEntityRecordFragment, TFragmentType>)
		/**
		 * @brief Default scoped builder for a record fragment.
		 * @tparam TFragmentType Fragment type being edited.
		 */
		struct TFragmentBuilderBase
		{
		public:
			/**
			 * @brief Creates a scoped fragment builder.
			 * @param InParent Parent entity-record builder.
			 * @param InFragment Fragment being edited.
			 */
			TFragmentBuilderBase(FBuilder& InParent, TFragmentType& InFragment)
				: Parent(InParent)
				, Fragment(InFragment)
			{
			}

			/** Provides pointer-style access to the fragment. */
			FORCEINLINE TFragmentType* operator->()
			{
				return &Fragment;
			}
			
			/** @return The fragment being edited. */
			NO_DISCARD FORCEINLINE TFragmentType& GetSelf()
			{
				return Fragment;
			}
			
			/**
			 * @brief Ends the fragment scope.
			 * @return The parent entity-record builder.
			 */
			FORCEINLINE FBuilder& End() const
			{
				return Parent;
			}

		private:
			FBuilder& Parent;
			TFragmentType& Fragment;
			
		}; // struct TFragmentBuilderBase
		
		template <UE::Flecs::CNonStructUtilScriptStructType TFragmentType, typename... TArgs>
		requires (std::is_base_of_v<FFlecsEntityRecordFragment, TFragmentType>)
		/**
		 * @brief Opens a scope for configuring a record fragment.
		 *
		 * Uses a fragment-provided custom builder when one is available; otherwise it
		 * returns the default fragment builder.
		 *
		 * @tparam TFragmentType Fragment type derived from FFlecsEntityRecordFragment.
		 * @tparam TArgs Constructor argument types.
		 * @param InArgs Arguments forwarded to the fragment or custom builder.
		 * @return A scoped fragment builder.
		 */
		FORCEINLINE auto FragmentScope(TArgs&&... InArgs)
		{
			TFragmentType& Fragment = EntityRecord.GetOrAddFragment<TFragmentType>(Forward<TArgs>(InArgs)...);

			if constexpr (UE::Flecs::Entity::Records::CHasCustomFragmentBuilder<FBuilder, TFragmentType>)
			{
				return typename TFragmentType::FBuilder(*this, Fragment);
			}
			else
			{
				return TFragmentBuilderBase<TFragmentType>(*this, Fragment);
			}
		}
		
		/**
		 * @brief Scoped builder for a child entity record.
		 */
		struct FSubEntityScope
		{
			/**
			 * @brief Creates a scoped child-record builder.
			 * @param InParent Parent entity-record builder.
			 * @param InChildRecord Child record being edited.
			 */
			FSubEntityScope(FBuilder& InParent, FFlecsEntityRecord& InChildRecord)
				: Parent(&InParent)
				, ChildRecord(&InChildRecord)
			{
			}
			
			/** Provides pointer-style access to the child-record builder. */
			FORCEINLINE FBuilder operator->() const
			{
				return FBuilder(*ChildRecord);
			}
			
			/** @return A builder for the child record. */
			NO_DISCARD FORCEINLINE FBuilder Get() const
			{
				return FBuilder(*ChildRecord);
			}
			
			/**
			 * @brief Ends the child-record scope.
			 * @return The parent entity-record builder.
			 */
			FORCEINLINE FBuilder& End() const
			{
				return *Parent;
			}

		private:
			FBuilder* Parent = nullptr;
			FFlecsEntityRecord* ChildRecord = nullptr;
			
		}; // struct FSubEntityScope
		
		/**
		 * @brief Opens a scope for constructing a new child entity record.
		 * @param bInDontFragmentParentChildRelationship Whether to avoid fragmenting the parent-child relationship when applied.
		 * @return A scoped builder for the new child record.
		 */
		FORCEINLINE FSubEntityScope SubEntity(const bool bInDontFragmentParentChildRelationship = true)
		{
			const int32 Index =
				EntityRecord.SubEntities.Add(FFlecsSubEntityRecord(
					TInstancedStruct<FFlecsEntityRecord>::Make<FFlecsEntityRecord>(),
					bInDontFragmentParentChildRelationship));

			FFlecsEntityRecord& Child = EntityRecord.SubEntities[Index].Record.GetMutable();
			return FSubEntityScope(*this, Child);
		}
		
		/**
		 * @brief Adds an existing child entity record.
		 * @param InSubEntity Child record to copy.
		 * @return This builder.
		 */
		FORCEINLINE FBuilder& SubEntity(const FFlecsEntityRecord& InSubEntity)
		{
			EntityRecord.AddSubEntity(InSubEntity);
			return *this;
		}
		
	private:
		FFlecsEntityRecord& EntityRecord;
		
	}; // struct FBuilder
	
	template <UE::Flecs::CNonStructUtilScriptStructType TFragmentType>
	/**
	 * @brief Convenience alias for the default builder of a fragment type.
	 * @tparam TFragmentType Fragment type being configured.
	 */
	using TFragmentBuilderType = FBuilder::TFragmentBuilderBase<TFragmentType>;
	
	/**
	 * @brief Creates a fluent builder for this record.
	 * @return A builder referencing this record.
	 */
	NO_DISCARD FORCEINLINE FBuilder Builder()
	{
		return FBuilder(*this);
	}

	/**
	 * @brief Compares all component, child-record, and fragment data.
	 * @param Other Record to compare with.
	 * @return True when the records contain equal data.
	 */
	NO_DISCARD FORCEINLINE bool UEOpEquals(const FFlecsEntityRecord& Other) const
	{
		return Components == Other.Components && SubEntities == Other.SubEntities && Fragments == Other.Fragments;
	}
	
	/**
	 * @brief Adds a reflected script-struct type without an instance value.
	 * @param InStructType Script-struct type to add.
	 * @return This record.
	 */
	FORCEINLINE FFlecsEntityRecord& AddComponent(const TSolidNotNull<const UScriptStruct*> InStructType)
	{
		FFlecsComponentTypeInfo NewComponent;
		NewComponent.NodeType = EFlecsComponentNodeType::ScriptStruct;

		FInstancedStruct NewInstancedStruct;
		NewInstancedStruct.InitializeAs(InStructType);
		
		NewComponent.ScriptStruct = MoveTemp(NewInstancedStruct);
		Components.Add(NewComponent);

		return *this;
	}
	
	/**
	 * @brief Adds a copied instanced-struct component.
	 * @param InStruct Struct value to copy.
	 * @return This record.
	 */
	FORCEINLINE FFlecsEntityRecord& AddComponent(const FInstancedStruct& InStruct)
	{
		FFlecsComponentTypeInfo NewComponent;
		NewComponent.NodeType = EFlecsComponentNodeType::ScriptStruct;
		NewComponent.ScriptStruct = InStruct;
		Components.Add(NewComponent);

		return *this;
	}
	
	/**
	 * @brief Adds a moved instanced-struct component.
	 * @param InStruct Struct value to move.
	 * @return This record.
	 */
	FORCEINLINE FFlecsEntityRecord& AddComponent(FInstancedStruct&& InStruct)
	{
		FFlecsComponentTypeInfo NewComponent;
		NewComponent.NodeType = EFlecsComponentNodeType::ScriptStruct;
		NewComponent.ScriptStruct = MoveTemp(InStruct);
		Components.Add(NewComponent);

		return *this;
	}

	template <UE::Flecs::CNonStructUtilScriptStructType T>
	requires (!std::is_enum_v<T>)
	/**
	 * @brief Adds a default-initialized typed script-struct component.
	 * @tparam T Script-struct component type.
	 * @return This record.
	 */
	FORCEINLINE FFlecsEntityRecord& AddComponent()
	{
		FFlecsComponentTypeInfo NewComponent;
		NewComponent.NodeType = EFlecsComponentNodeType::ScriptStruct;
		NewComponent.ScriptStruct = FInstancedStruct::Make<T>();
		Components.Add(NewComponent);

		return *this;
	}

	template <UE::Flecs::CNonStructUtilScriptStructType T>
	requires (!std::is_enum_v<T>)
	/**
	 * @brief Adds a copied typed script-struct component.
	 * @tparam T Script-struct component type.
	 * @param InComponent Component value to copy.
	 * @return This record.
	 */
	FORCEINLINE FFlecsEntityRecord& AddComponent(const T& InComponent)
	{
		FFlecsComponentTypeInfo NewComponent;
		NewComponent.NodeType = EFlecsComponentNodeType::ScriptStruct;
		NewComponent.ScriptStruct = FInstancedStruct::Make<T>(InComponent);
		Components.Add(NewComponent);

		return *this;
	}

	template <UE::Flecs::CNonStructUtilScriptStructType T>
	requires (!std::is_enum_v<T>)
	/**
	 * @brief Adds a moved typed script-struct component.
	 * @tparam T Script-struct component type.
	 * @param InComponent Component value to move.
	 * @return This record.
	 */
	FORCEINLINE FFlecsEntityRecord& AddComponent(T&& InComponent)
	{
		FFlecsComponentTypeInfo NewComponent;
		NewComponent.NodeType = EFlecsComponentNodeType::ScriptStruct;
		NewComponent.ScriptStruct = FInstancedStruct::Make<T>(MoveTemp(InComponent));
		Components.Add(NewComponent);

		return *this;
	}

	/**
	 * @brief Adds an entity identifier as a component.
	 * @param InEntityHandle Entity identifier to add.
	 * @return This record.
	 */
	FORCEINLINE FFlecsEntityRecord& AddComponent(const FFlecsId InEntityHandle)
	{
		FFlecsComponentTypeInfo NewComponent;
		NewComponent.NodeType = EFlecsComponentNodeType::EntityHandle;
		NewComponent.EntityHandle = InEntityHandle;
		Components.Add(NewComponent);

		return *this;
	}

	/**
	 * @brief Adds a gameplay tag as a component.
	 * @param InGameplayTag Gameplay tag to add.
	 * @return This record.
	 */
	FORCEINLINE FFlecsEntityRecord& AddComponent(const FGameplayTag& InGameplayTag)
	{
		FFlecsComponentTypeInfo NewComponent;
		NewComponent.NodeType = EFlecsComponentNodeType::FGameplayTag;
		NewComponent.GameplayTag = InGameplayTag;
		Components.Add(NewComponent);

		return *this;
	}

	/**
	 * @brief Adds a moved gameplay tag as a component.
	 * @param InGameplayTag Gameplay tag to move.
	 * @return This record.
	 */
	FORCEINLINE FFlecsEntityRecord& AddComponent(FGameplayTag&& InGameplayTag)
	{
		FFlecsComponentTypeInfo NewComponent;
		NewComponent.NodeType = EFlecsComponentNodeType::FGameplayTag;
		NewComponent.GameplayTag = MoveTemp(InGameplayTag);
		Components.Add(NewComponent);

		return *this;
	}

	/**
	 * @brief Adds a reflected script-enum value as a component.
	 * @param InScriptEnum Enum selector to copy.
	 * @return This record.
	 */
	FORCEINLINE FFlecsEntityRecord& AddComponent(const FSolidEnumSelector& InScriptEnum)
	{
		FFlecsComponentTypeInfo NewComponent;
		NewComponent.NodeType = EFlecsComponentNodeType::ScriptEnum;
		NewComponent.ScriptEnum = InScriptEnum;
		Components.Add(NewComponent);

		return *this;
	}

	/**
	 * @brief Adds a moved script-enum value as a component.
	 * @param InScriptEnum Enum selector to move.
	 * @return This record.
	 */
	FORCEINLINE FFlecsEntityRecord& AddComponent(FSolidEnumSelector&& InScriptEnum)
	{
		FFlecsComponentTypeInfo NewComponent;
		NewComponent.NodeType = EFlecsComponentNodeType::ScriptEnum;
		NewComponent.ScriptEnum = MoveTemp(InScriptEnum);
		Components.Add(NewComponent);

		return *this;
	}
	
	template <Solid::TStaticEnumConcept TEnum>
	requires (std::is_enum_v<TEnum>)
	/**
	 * @brief Adds a native enum value as a component.
	 * @tparam TEnum Registered native enum type.
	 * @param InEnumValue Enum value to add.
	 * @return This record.
	 */
	FORCEINLINE FFlecsEntityRecord& AddComponent(const TEnum InEnumValue)
	{
		FFlecsComponentTypeInfo NewComponent;
		NewComponent.NodeType = EFlecsComponentNodeType::ScriptEnum;
		
		FSolidEnumSelector NewEnumSelector;
		NewEnumSelector.Class = StaticEnum<TEnum>();
		NewEnumSelector.Value = static_cast<int64>(InEnumValue);
		
		NewComponent.ScriptEnum = NewEnumSelector;
		
		Components.Add(NewComponent);
		
		return *this;
	}

	/**
	 * @brief Adds a copied record pair as a component.
	 * @param InPair Pair to copy.
	 * @return This record.
	 */
	FORCEINLINE FFlecsEntityRecord& AddComponent(const FFlecsRecordPair& InPair)
	{
		FFlecsComponentTypeInfo NewComponent;
		NewComponent.NodeType = EFlecsComponentNodeType::Pair;
		NewComponent.Pair = InPair;
		Components.Add(NewComponent);

		return *this;
	}

	/**
	 * @brief Adds a moved record pair as a component.
	 * @param InPair Pair to move.
	 * @return This record.
	 */
	FORCEINLINE FFlecsEntityRecord& AddComponent(FFlecsRecordPair&& InPair)
	{
		FFlecsComponentTypeInfo NewComponent;
		NewComponent.NodeType = EFlecsComponentNodeType::Pair;
		NewComponent.Pair = MoveTemp(InPair);
		Components.Add(NewComponent);

		return *this;
	}

	/**
	 * @brief Appends a child entity record.
	 * @param InSubEntity Child record to copy.
	 * @param bInDontFragmentParentChildRelationship Whether to avoid fragmenting the parent-child relationship when applied.
	 * @return Index of the newly added child record.
	 */
	FORCEINLINE int32 AddSubEntity(const FFlecsEntityRecord& InSubEntity, const bool bInDontFragmentParentChildRelationship = true)
	{
		return SubEntities.Add(FFlecsSubEntityRecord(
			TInstancedStruct<FFlecsEntityRecord>::Make(InSubEntity),
			bInDontFragmentParentChildRelationship));
	}

	/**
	 * @brief Removes one child entity record.
	 * @param InIndex Index of the child record to remove.
	 */
	FORCEINLINE void RemoveSubEntity(const int32 InIndex)
	{
		solid_checkf(SubEntities.IsValidIndex(InIndex), TEXT("Index is out of bounds"));
		SubEntities.RemoveAt(InIndex);
	}

	/** Removes all child entity records. */
	FORCEINLINE void RemoveAllSubEntities()
	{
		SubEntities.Empty();
	}

	/** @return True when this record contains at least one child record. */
	NO_DISCARD FORCEINLINE bool HasSubEntities() const
	{
		return !SubEntities.IsEmpty();
	}

	/** @return Number of child entity records in this record. */
	NO_DISCARD FORCEINLINE int32 GetSubEntityCount() const
	{
		return SubEntities.Num();
	}

	/**
	 * @brief Gets a const view of a child entity record.
	 * @param InIndex Child-record index.
	 * @return Const view of the selected child record.
	 */
	NO_DISCARD FORCEINLINE TConstStructView<FFlecsEntityRecord> GetSubEntity(const int32 InIndex) const
	{
		solid_checkf(SubEntities.IsValidIndex(InIndex), TEXT("Index is out of bounds"));
		return SubEntities[InIndex].Record;
	}

	/**
	 * @brief Gets a mutable view of a child entity record.
	 * @param InIndex Child-record index.
	 * @return Mutable view of the selected child record.
	 */
	NO_DISCARD FORCEINLINE TStructView<FFlecsEntityRecord> GetSubEntity(const int32 InIndex)
	{
		solid_checkf(SubEntities.IsValidIndex(InIndex), TEXT("Index is out of bounds"));
		return SubEntities[InIndex].Record;
	}

	template <UE::Flecs::CNonStructUtilScriptStructType TFragmentType>
	requires (std::is_base_of_v<FFlecsEntityRecordFragment, TFragmentType>)
	/**
	 * @brief Adds a copy of a typed record fragment.
	 * @tparam TFragmentType Fragment type derived from FFlecsEntityRecordFragment.
	 * @param InFragment Fragment value to copy.
	 * @return Index of the new fragment, or INDEX_NONE when a duplicate exists.
	 */
	FORCEINLINE int32 AddFragment(const TFragmentType& InFragment)
	{
		if UNLIKELY_IF(!ensureAlwaysMsgf(!HasFragment<TFragmentType>(),
			TEXT("Fragment of type %s already exists in Entity Record, adding duplicate fragment."),
			*TBaseStructure<TFragmentType>::Get()->GetName()))
		{
			return INDEX_NONE;
		}
		
		return Fragments.Add(TInstancedStruct<FFlecsEntityRecordFragment>::Make<TFragmentType>(InFragment));
	}

	template <UE::Flecs::CNonStructUtilScriptStructType TFragmentType, typename... TArgs>
	requires (std::is_base_of_v<FFlecsEntityRecordFragment, TFragmentType>)
	/**
	 * @brief Constructs and adds a typed record fragment.
	 * @tparam TFragmentType Fragment type derived from FFlecsEntityRecordFragment.
	 * @tparam TArgs Constructor argument types.
	 * @param InArgs Arguments forwarded to the fragment constructor.
	 * @return Index of the new fragment, or INDEX_NONE when a duplicate exists.
	 */
	FORCEINLINE int32 AddFragment(TArgs&&... InArgs)
	{
		if UNLIKELY_IF(!ensureAlwaysMsgf(!HasFragment<TFragmentType>(),
			TEXT("Fragment of type %s already exists in Entity Record, adding duplicate fragment."),
			*TBaseStructure<TFragmentType>::Get()->GetName()))
		{
			return INDEX_NONE;
		}
		
		return Fragments.Add(TInstancedStruct<FFlecsEntityRecordFragment>::Make<TFragmentType>(Forward<TArgs>(InArgs)...));
	}

	template <UE::Flecs::CNonStructUtilScriptStructType TFragmentType>
	requires (std::is_base_of_v<FFlecsEntityRecordFragment, TFragmentType>)
	/**
	 * @brief Default-constructs and adds a typed record fragment.
	 * @tparam TFragmentType Fragment type derived from FFlecsEntityRecordFragment.
	 * @return Index of the new fragment, or INDEX_NONE when a duplicate exists.
	 */
	FORCEINLINE int32 AddFragment()
	{
		if UNLIKELY_IF(!ensureAlwaysMsgf(!HasFragment<TFragmentType>(),
			TEXT("Fragment of type %s already exists in Entity Record, adding duplicate fragment."),
			*TBaseStructure<TFragmentType>::Get()->GetName()))
		{
			return INDEX_NONE;
		}
		
		return Fragments.Add(TInstancedStruct<FFlecsEntityRecordFragment>::Make<TFragmentType>());
	}

	/**
	 * @brief Adds an instanced record fragment.
	 * @param InFragment Fragment instance to copy.
	 * @return Index of the new fragment, or INDEX_NONE when a duplicate exists.
	 */
	FORCEINLINE int32 AddFragment(const TInstancedStruct<FFlecsEntityRecordFragment>& InFragment)
	{
		solid_checkf(InFragment.IsValid(), TEXT("InFragment must be valid"));
		
		if UNLIKELY_IF(!ensureAlwaysMsgf(!HasFragment(InFragment.GetScriptStruct()),
			TEXT("Fragment of type %s already exists in Entity Record, adding duplicate fragment."),
			*InFragment.GetScriptStruct()->GetName()))
		{
			return INDEX_NONE;
		}
		
		return Fragments.Add(InFragment);
	}
	
	template <Solid::TScriptStructConcept TFragmentType>
	requires (std::is_base_of_v<FFlecsEntityRecordFragment, TFragmentType>)
	/**
	 * @brief Gets an existing typed fragment or adds a default one.
	 * @tparam TFragmentType Fragment type derived from FFlecsEntityRecordFragment.
	 * @return Mutable reference to the fragment.
	 */
	FORCEINLINE TFragmentType& GetOrAddFragment()
	{
		for (TInstancedStruct<FFlecsEntityRecordFragment>& Fragment : Fragments)
		{
			if (Fragment.GetScriptStruct() == TBaseStructure<TFragmentType>::Get())
			{
				return Fragment.GetMutable<TFragmentType>();
			}
		}

		const int32 NewIndex = AddFragment<TFragmentType>();
		solid_cassumef(NewIndex != INDEX_NONE, TEXT("Failed to add new fragment"));
		
		return Fragments[NewIndex].GetMutable<TFragmentType>();
	}
	
	template <Solid::TScriptStructConcept TFragmentType, typename... TArgs>
	requires (std::is_base_of_v<FFlecsEntityRecordFragment, TFragmentType>)
	/**
	 * @brief Gets an existing typed fragment or constructs one.
	 * @tparam TFragmentType Fragment type derived from FFlecsEntityRecordFragment.
	 * @tparam TArgs Constructor argument types.
	 * @param InArgs Arguments forwarded when a new fragment is created.
	 * @return Mutable reference to the fragment.
	 */
	FORCEINLINE TFragmentType& GetOrAddFragment(TArgs&&... InArgs)
	{
		for (TInstancedStruct<FFlecsEntityRecordFragment>& Fragment : Fragments)
		{
			if (Fragment.GetScriptStruct() == TBaseStructure<TFragmentType>::Get())
			{
				return Fragment.GetMutable<TFragmentType>();
			}
		}
		
		const int32 NewIndex = AddFragment<TFragmentType>(Forward<TArgs>(InArgs)...);
		solid_cassumef(NewIndex != INDEX_NONE, TEXT("Failed to add new fragment"));
		
		return Fragments[NewIndex].GetMutable<TFragmentType>();
	}

	/**
	 * @brief Tests whether a fragment type is present.
	 * @param InFragmentType Fragment script-struct type to find.
	 * @return True when the record contains that fragment type.
	 */
	NO_DISCARD FORCEINLINE bool HasFragment(const TSolidNotNull<const UScriptStruct*> InFragmentType) const
	{
		return Fragments.ContainsByPredicate(
			[InFragmentType](const TInstancedStruct<FFlecsEntityRecordFragment>& Fragment)
			{
				return Fragment.GetScriptStruct() == InFragmentType;
			});
	}

	template <Solid::TScriptStructConcept TFragmentType>
	requires (std::is_base_of_v<FFlecsEntityRecordFragment, TFragmentType>)
	/**
	 * @brief Tests whether a typed fragment is present.
	 * @tparam TFragmentType Fragment type to find.
	 * @return True when the record contains that fragment type.
	 */
	NO_DISCARD FORCEINLINE bool HasFragment() const
	{
		return HasFragment(TBaseStructure<TFragmentType>::Get());
	}
	
	/** @return True when this record contains at least one fragment. */
	NO_DISCARD FORCEINLINE bool HasFragments() const
	{
		return !Fragments.IsEmpty();
	}

	/** @return Number of fragments in this record. */
	NO_DISCARD FORCEINLINE int32 GetFragmentCount() const
	{
		return Fragments.Num();
	}

	/**
	 * @brief Gets a const view of a fragment by index.
	 * @param InIndex Fragment index.
	 * @return Const view of the selected fragment.
	 */
	NO_DISCARD FORCEINLINE TConstStructView<FFlecsEntityRecordFragment> GetFragment(const int32 InIndex) const
	{
		solid_checkf(Fragments.IsValidIndex(InIndex), TEXT("Index is out of bounds"));
		return Fragments[InIndex];
	}

	/**
	 * @brief Gets a mutable view of a fragment by index.
	 * @param InIndex Fragment index.
	 * @return Mutable view of the selected fragment.
	 */
	NO_DISCARD FORCEINLINE TStructView<FFlecsEntityRecordFragment> GetFragment(const int32 InIndex)
	{
		solid_checkf(Fragments.IsValidIndex(InIndex), TEXT("Index is out of bounds"));
		return Fragments[InIndex];
	}
	
	template <Solid::TScriptStructConcept TFragmentType>
	requires (std::is_base_of_v<FFlecsEntityRecordFragment, TFragmentType>)
	/**
	 * @brief Gets a mutable typed fragment by index.
	 * @tparam TFragmentType Expected fragment type.
	 * @param InIndex Fragment index.
	 * @return Mutable reference to the selected fragment.
	 */
	NO_DISCARD FORCEINLINE TFragmentType& GetFragment(const int32 InIndex)
	{
		solid_checkf(Fragments.IsValidIndex(InIndex), TEXT("Index is out of bounds"));
		
		TInstancedStruct<FFlecsEntityRecordFragment>& Fragment = Fragments[InIndex];
		solid_checkf(Fragment.GetScriptStruct() == TBaseStructure<TFragmentType>::Get(),
			TEXT("Fragment at index %d is not of type %s"), InIndex,
			*TBaseStructure<TFragmentType>::Get()->GetName());
		
		return Fragment.GetMutable<TFragmentType>();
	}
	
	template <Solid::TScriptStructConcept TFragmentType>
	requires (std::is_base_of_v<FFlecsEntityRecordFragment, TFragmentType>)
	/**
	 * @brief Gets a const typed fragment by index.
	 * @tparam TFragmentType Expected fragment type.
	 * @param InIndex Fragment index.
	 * @return Const reference to the selected fragment.
	 */
	NO_DISCARD FORCEINLINE const TFragmentType& GetFragment(const int32 InIndex) const
	{
		solid_checkf(Fragments.IsValidIndex(InIndex), TEXT("Index is out of bounds"));
		
		const TInstancedStruct<FFlecsEntityRecordFragment>& Fragment = Fragments[InIndex];
		solid_checkf(Fragment.GetScriptStruct() == TBaseStructure<TFragmentType>::Get(),
			TEXT("Fragment at index %d is not of type %s"), InIndex,
			*TBaseStructure<TFragmentType>::Get()->GetName());
		
		return Fragment.Get<TFragmentType>();
	}
	
	/*template <Solid::TScriptStructConcept TFragmentType>
	requires (std::is_base_of_v<FFlecsEntityRecordFragment, TFragmentType>)
	NO_DISCARD FORCEINLINE TFragmentType& GetFragment()
	{
		for (TInstancedStruct<FFlecsEntityRecordFragment>& Fragment : Fragments)
		{
			if (Fragment.GetScriptStruct() == TBaseStructure<TFragmentType>::Get())
			{
				return TStructView<TFragmentType>(Fragment);
			}
		}
		
		return TStructView<TFragmentType>();
	}
	
	template <Solid::TScriptStructConcept TFragmentType>
	requires (std::is_base_of_v<FFlecsEntityRecordFragment, TFragmentType>)
	NO_DISCARD FORCEINLINE TFragmentType& GetFragment() const
	{
		for (const TInstancedStruct<FFlecsEntityRecordFragment>& Fragment : Fragments)
		{
			if (Fragment.GetScriptStruct() == TBaseStructure<TFragmentType>::Get())
			{
				return TConstStructView<TFragmentType>(Fragment);
			}
		}
		
		return TConstStructView<TFragmentType>();
	}*/

	/**
	 * @brief Applies this record to an entity using its world context.
	 * @param InEntityHandle Entity that receives the record.
	 */
	void ApplyRecordToEntity(const FFlecsEntityHandle& InEntityHandle) const;
	/**
	 * @brief Applies this record to an entity in an explicit Flecs world.
	 * @param InFlecsWorld World interface used for record application.
	 * @param InEntityHandle Entity that receives the record.
	 */
	void ApplyRecordToEntity(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InFlecsWorld, const FFlecsEntityHandle& InEntityHandle) const;

}; // struct FFlecsEntityRecord


