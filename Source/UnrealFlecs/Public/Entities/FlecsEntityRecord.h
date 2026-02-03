// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "StructUtils/InstancedStruct.h"

#include "Concepts/SolidConcepts.h"
#include "Types/SolidEnumSelector.h"

#include "Entities/FlecsEntityHandle.h"
#include "Properties/FlecsComponentProperties.h"

#include "FlecsEntityRecord.generated.h"

struct FFlecsEntityRecord;

UENUM(BlueprintType)
enum class EFlecsComponentNodeType : uint8
{
	ScriptStruct = 0,
	EntityHandle = 1,
	FGameplayTag = 2,
	Pair = 3, /* All Pairs if both are component types then the first type is assumed as value */
	ScriptEnum = 4,
}; // enum class EFlecsComponentNodeType

UENUM(BlueprintType)
enum class EFlecsPairNodeType : uint8
{
	ScriptStruct = 0,
	EntityHandle = 1,
	FGameplayTag = 2,
	//ScriptEnum = 3,
}; // enum class EFlecsPairNodeType

namespace Unreal::Flecs
{
	template <typename T>
	concept CRecordPairSlotType = std::is_convertible<T, FFlecsId>::value
		|| std::is_convertible<T, FGameplayTag>::value
		|| std::is_convertible<T, FInstancedStruct>::value;
	
	template <typename T>
	concept CNonStructUtilScriptStructType = Solid::TScriptStructConcept<T> && !Solid::TStructUtilsTypeConcept<T>;
		
	namespace Entity::Records
	{
		template <typename TParentBuilder, typename TFragmentType>
		concept CHasCustomFragmentBuilder = requires(TParentBuilder& Parent, TFragmentType& InFragment)
		{
			typename TFragmentType::FBuilder;
			{ typename TFragmentType::FBuilder(Parent, InFragment) };
			
		}; // concept CHasCustomFragmentBuilder
		
	} // namespace Entity::Records
	
} // namespace Unreal::Flecs

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsRecordPairSlot
{
	GENERATED_BODY()

	static NO_DISCARD FFlecsRecordPairSlot Make(const FInstancedStruct& InStruct)
	{
		FFlecsRecordPairSlot OutSlot;
		OutSlot.PairNodeType = EFlecsPairNodeType::ScriptStruct;
		OutSlot.PairScriptStruct = InStruct;
		return OutSlot;
	}

	static NO_DISCARD FFlecsRecordPairSlot Make(const TSolidNotNull<const UScriptStruct*> InStructType)
	{
		FFlecsRecordPairSlot OutSlot;
		OutSlot.PairNodeType = EFlecsPairNodeType::ScriptStruct;

		FInstancedStruct NewInstancedStruct;
		NewInstancedStruct.InitializeAs(InStructType);
		
		OutSlot.PairScriptStruct = MoveTemp(NewInstancedStruct);
		return OutSlot;
	}

	template <Unreal::Flecs::CNonStructUtilScriptStructType T>
	static NO_DISCARD FFlecsRecordPairSlot Make()
	{
		FFlecsRecordPairSlot OutSlot;
		OutSlot.PairNodeType = EFlecsPairNodeType::ScriptStruct;
		OutSlot.PairScriptStruct = FInstancedStruct::Make<T>();
		return OutSlot;
	}

	template <Unreal::Flecs::CNonStructUtilScriptStructType T>
	static NO_DISCARD FFlecsRecordPairSlot Make(const T& InValue)
	{
		FFlecsRecordPairSlot OutSlot;
		OutSlot.PairNodeType = EFlecsPairNodeType::ScriptStruct;
		OutSlot.PairScriptStruct = FInstancedStruct::Make<T>(InValue);
		return OutSlot;
	}
	
	static NO_DISCARD FFlecsRecordPairSlot Make(const FFlecsId InEntityHandle)
	{
		FFlecsRecordPairSlot OutSlot;
		OutSlot.PairNodeType = EFlecsPairNodeType::EntityHandle;
		OutSlot.EntityHandle = InEntityHandle;
		return OutSlot;
	}

	static NO_DISCARD FFlecsRecordPairSlot Make(const FGameplayTag& InGameplayTag)
	{
		FFlecsRecordPairSlot OutSlot;
		OutSlot.PairNodeType = EFlecsPairNodeType::FGameplayTag;
		OutSlot.GameplayTag = InGameplayTag;
		return OutSlot;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component Tree")
	EFlecsPairNodeType PairNodeType = EFlecsPairNodeType::ScriptStruct;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component Tree",
		meta = (EditCondition = "PairNodeType == EFlecsPairNodeType::ScriptStruct", EditConditionHides))
	FInstancedStruct PairScriptStruct;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component Tree",
	//	meta = (EditCondition = "PairNodeType == EFlecsPairNodeType::ScriptEnum", EditConditionHides))
	//FSolidEnumSelector PairScriptEnum;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component Tree",
		meta = (EditCondition = "PairNodeType == EFlecsPairNodeType::EntityHandle", EditConditionHides))
	FFlecsId EntityHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component Tree",
		meta = (EditCondition = "PairNodeType == EFlecsPairNodeType::FGameplayTag", EditConditionHides))
	FGameplayTag GameplayTag;

	NO_DISCARD FORCEINLINE bool operator==(const FFlecsRecordPairSlot& Other) const
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

		UNREACHABLE;
	}

	NO_DISCARD FORCEINLINE bool operator!=(const FFlecsRecordPairSlot& Other) const
	{
		return !(*this == Other);
	}
	
}; // struct FFlecsPairSlot

UENUM(BlueprintType)
enum class EFlecsValuePairType : uint8
{
	None = 0,
	First = 1,
	Second = 2,
}; // enum class EFlecsValuePairType

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsRecordPair
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component Tree")
	FFlecsRecordPairSlot First;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component Tree")
	FFlecsRecordPairSlot Second;

	// @TODO: make an edit condition for this?
	/**
	 * @brief Only needed if either First or Second are Instanced Structs
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component Tree",
		meta = (EditCondition = "First.PairNodeType == EFlecsPairNodeType::ScriptStruct || Second.PairNodeType == EFlecsPairNodeType::ScriptStruct",
		EditConditionHides))
	EFlecsValuePairType PairValueType = EFlecsValuePairType::None;

	NO_DISCARD FORCEINLINE bool operator==(const FFlecsRecordPair& Other) const
	{
		return First == Other.First && Second == Other.Second;
	}

	NO_DISCARD FORCEINLINE bool operator!=(const FFlecsRecordPair& Other) const
	{
		return !(*this == Other);
	}
	
	void AddToEntity(const FFlecsEntityHandle& InEntityHandle) const;

private:
	
	template <Unreal::Flecs::CRecordPairSlotType T>
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
struct UNREALFLECS_API FFlecsComponentTypeInfo final
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component Tree")
	EFlecsComponentNodeType NodeType = EFlecsComponentNodeType::ScriptStruct;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component Tree",
		meta = (EditCondition = "NodeType == EFlecsComponentNodeType::ScriptStruct", EditConditionHides))
	FInstancedStruct ScriptStruct;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component Tree",
		meta = (EditCondition = "NodeType == EFlecsComponentNodeType::ScriptEnum", EditConditionHides))
	FSolidEnumSelector ScriptEnum;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component Tree",
		meta = (EditCondition = "NodeType == EFlecsComponentNodeType::EntityHandle", EditConditionHides))
	FFlecsId EntityHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component Tree",
		meta = (EditCondition = "NodeType == EFlecsComponentNodeType::FGameplayTag", EditConditionHides))
	FGameplayTag GameplayTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component Tree",
		meta = (EditCondition = "NodeType == EFlecsComponentNodeType::Pair", EditConditionHides))
	FFlecsRecordPair Pair;

	NO_DISCARD FORCEINLINE bool operator==(const FFlecsComponentTypeInfo& Other) const
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

		UNREACHABLE;
	}

	NO_DISCARD FORCEINLINE bool operator!=(const FFlecsComponentTypeInfo& Other) const
	{
		return !(*this == Other);
	}
	
}; // struct FFlecsComponentTypeInfo

// @TODO: utilize C++23 'explicit this' feature when available

USTRUCT(BlueprintInternalUseOnly)
struct UNREALFLECS_API FFlecsEntityRecordFragment
{
	GENERATED_BODY()

public:
	FFlecsEntityRecordFragment() = default;
	
	virtual ~FFlecsEntityRecordFragment()
	{
	}

	virtual void PreApplyRecordToEntity(const TSolidNotNull<const UFlecsWorld*> InFlecsWorld, const FFlecsEntityHandle& InEntityHandle) const {}
	virtual void PostApplyRecordToEntity(const TSolidNotNull<const UFlecsWorld*> InFlecsWorld, const FFlecsEntityHandle& InEntityHandle) const {}
	
}; // struct FFlecsEntityRecordFragment

// @TODO: maybe align on a 64-byte boundary?
// @TODO: make custom UI for FFlecsEntityRecord and make custom meta tags.
// @TODO: add data validation.

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsSubEntityRecord
{
	GENERATED_BODY()
	
public:
	FORCEINLINE FFlecsSubEntityRecord() = default;
	
	FORCEINLINE explicit FFlecsSubEntityRecord(const TInstancedStruct<FFlecsEntityRecord>& InRecord, 
		const bool bInDontFragmentParentChildRelationship = true)
		: bDontFragmentParentChildRelationship(bInDontFragmentParentChildRelationship)
		, Record(InRecord)
	{
	}
	
	FORCEINLINE explicit FFlecsSubEntityRecord(TInstancedStruct<FFlecsEntityRecord>&& InRecord, 
		const bool bInDontFragmentParentChildRelationship = true)
		: bDontFragmentParentChildRelationship(bInDontFragmentParentChildRelationship)
		, Record(MoveTemp(InRecord))
	{
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity Record")
	bool bDontFragmentParentChildRelationship = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity Record")
	TInstancedStruct<FFlecsEntityRecord> Record;
	
	NO_DISCARD FORCEINLINE bool operator==(const FFlecsSubEntityRecord& Other) const
	{
		return bDontFragmentParentChildRelationship == Other.bDontFragmentParentChildRelationship
			&& Record == Other.Record;
	}
	
	NO_DISCARD FORCEINLINE bool operator!=(const FFlecsSubEntityRecord& Other) const
	{
		return !(*this == Other);
	}
	
}; // struct FFlecsSubEntityRecord

/**
 * @brief A record of a generic entity's components and sub-entities,
 * this can be applied to an actual entity to give it the same components/sub-entities.
 */
USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsEntityRecord final
{
	GENERATED_BODY()
	
public:

	FORCEINLINE FFlecsEntityRecord() = default;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity Record")
	TArray<FFlecsComponentTypeInfo> Components;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity Record")
	TArray<FFlecsSubEntityRecord> SubEntities;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity Record", meta = (ExcludeBaseStruct, NoElementDuplicate))
	TArray<TInstancedStruct<FFlecsEntityRecordFragment>> Fragments;
	
	struct FBuilder
	{
	public:
		FORCEINLINE explicit FBuilder(FFlecsEntityRecord& InRecord)
			: EntityRecord(InRecord)
		{
		}
		
		FORCEINLINE FFlecsEntityRecord& Build()
		{
			return EntityRecord;
		}
		
		FORCEINLINE const FFlecsEntityRecord& Build() const
		{
			return EntityRecord;
		}
		
		template <Unreal::Flecs::CNonStructUtilScriptStructType T>
		FORCEINLINE FBuilder& Component()
		{
			EntityRecord.AddComponent<T>();
			return *this;
		}

		template <Unreal::Flecs::CNonStructUtilScriptStructType T>
		FORCEINLINE FBuilder& Component(const T& InValue)
		{
			EntityRecord.AddComponent<T>(InValue);
			return *this;
		}

		template <Unreal::Flecs::CNonStructUtilScriptStructType T>
		FORCEINLINE FBuilder& Component(T&& InValue)
		{
			EntityRecord.AddComponent<T>(MoveTemp(InValue));
			return *this;
		}
		
		FORCEINLINE FBuilder& Component(const FFlecsId InEntityHandle)
		{
			EntityRecord.AddComponent(InEntityHandle);
			return *this;
		}
		
		FORCEINLINE FBuilder& Component(const TSolidNotNull<const UScriptStruct*> InStructType)
		{
			EntityRecord.AddComponent(InStructType);
			return *this;
		}
		
		FORCEINLINE FBuilder& Component(const FInstancedStruct& InStruct)
		{
			EntityRecord.AddComponent(InStruct);
			return *this;
		}
		
		FORCEINLINE FBuilder& Component(FInstancedStruct&& InStruct)
		{
			EntityRecord.AddComponent(MoveTemp(InStruct));
			return *this;
		}

		FORCEINLINE FBuilder& GameplayTag(const FGameplayTag& InGameplayTag)
		{
			EntityRecord.AddComponent(InGameplayTag);
			return *this;
		}

		FORCEINLINE FBuilder& GameplayTag(FGameplayTag&& InGameplayTag)
		{
			EntityRecord.AddComponent(MoveTemp(InGameplayTag));
			return *this;
		}

		FORCEINLINE FBuilder& Enum(const FSolidEnumSelector& InScriptEnum)
		{
			EntityRecord.AddComponent(InScriptEnum);
			return *this;
		}

		FORCEINLINE FBuilder& Enum(FSolidEnumSelector&& InScriptEnum)
		{
			EntityRecord.AddComponent(MoveTemp(InScriptEnum));
			return *this;
		}
		
		template <Solid::TStaticEnumConcept TEnum>
		FORCEINLINE FBuilder& Enum(const TEnum InEnumValue)
		{
			EntityRecord.AddComponent<TEnum>(InEnumValue);
			return *this;
		}

		FORCEINLINE FBuilder& Pair(const FFlecsRecordPair& InPair)
		{
			EntityRecord.AddComponent(InPair);
			return *this;
		}

		FORCEINLINE FBuilder& Pair(FFlecsRecordPair&& InPair)
		{
			EntityRecord.AddComponent(MoveTemp(InPair));
			return *this;
		}
		
		template <Unreal::Flecs::CNonStructUtilScriptStructType TFragmentType, typename... TArgs>
		requires (std::is_base_of_v<FFlecsEntityRecordFragment, TFragmentType>)
		FORCEINLINE FBuilder& Fragment(TArgs&&... InArgs)
		{
			EntityRecord.AddFragment<TFragmentType>(Forward<TArgs>(InArgs)...);
			
			return *this;
		}
		
		FORCEINLINE FBuilder& Fragment(const TInstancedStruct<FFlecsEntityRecordFragment>& InFragment)
		{
			EntityRecord.AddFragment(InFragment);
			return *this;
		}
		
		template <Unreal::Flecs::CNonStructUtilScriptStructType TFragmentType>
		requires (std::is_base_of_v<FFlecsEntityRecordFragment, TFragmentType>)
		struct TFragmentBuilderBase
		{
		public:
			TFragmentBuilderBase(FBuilder& InParent, TFragmentType& InFragment)
				: Parent(InParent)
				, Fragment(InFragment)
			{
			}

			FORCEINLINE TFragmentType* operator->()
			{
				return &Fragment;
			}
			
			NO_DISCARD FORCEINLINE TFragmentType& GetSelf()
			{
				return Fragment;
			}
			
			FORCEINLINE FBuilder& End() const
			{
				return Parent;
			}

		private:
			FBuilder& Parent;
			TFragmentType& Fragment;
			
		}; // struct TFragmentBuilderBase
		
		template <Unreal::Flecs::CNonStructUtilScriptStructType TFragmentType, typename... TArgs>
		requires (std::is_base_of_v<FFlecsEntityRecordFragment, TFragmentType>)
		FORCEINLINE auto FragmentScope(TArgs&&... InArgs)
		{
			TFragmentType& Fragment = EntityRecord.GetOrAddFragment<TFragmentType>(Forward<TArgs>(InArgs)...);

			if constexpr (Unreal::Flecs::Entity::Records::CHasCustomFragmentBuilder<FBuilder, TFragmentType>)
			{
				return typename TFragmentType::FBuilder(*this, Fragment);
			}
			else
			{
				return TFragmentBuilderBase<TFragmentType>(*this, Fragment);
			}
		}
		
		struct FSubEntityScope
		{
			FSubEntityScope(FBuilder& InParent, FFlecsEntityRecord& InChildRecord)
				: Parent(&InParent)
				, ChildRecord(&InChildRecord)
			{
			}
			
			FORCEINLINE FBuilder operator->() const
			{
				return FBuilder(*ChildRecord);
			}
			
			NO_DISCARD FORCEINLINE FBuilder Get() const
			{
				return FBuilder(*ChildRecord);
			}
			
			FORCEINLINE FBuilder& End() const
			{
				return *Parent;
			}

		private:
			FBuilder* Parent = nullptr;
			FFlecsEntityRecord* ChildRecord = nullptr;
			
		}; // struct FSubEntityScope
		
		FORCEINLINE FSubEntityScope SubEntity(const bool bInDontFragmentParentChildRelationship = true)
		{
			const int32 Index =
				EntityRecord.SubEntities.Add(FFlecsSubEntityRecord(
					TInstancedStruct<FFlecsEntityRecord>::Make<FFlecsEntityRecord>(),
					bInDontFragmentParentChildRelationship));

			FFlecsEntityRecord& Child = EntityRecord.SubEntities[Index].Record.GetMutable();
			return FSubEntityScope(*this, Child);
		}
		
		FORCEINLINE FBuilder& SubEntity(const FFlecsEntityRecord& InSubEntity)
		{
			EntityRecord.AddSubEntity(InSubEntity);
			return *this;
		}
		
	private:
		FFlecsEntityRecord& EntityRecord;
		
	}; // struct FBuilder
	
	template <Unreal::Flecs::CNonStructUtilScriptStructType TFragmentType>
	using FFragmentBuilderType = FBuilder::TFragmentBuilderBase<TFragmentType>;
	
	FORCEINLINE FBuilder Builder()
	{
		return FBuilder(*this);
	}

	NO_DISCARD FORCEINLINE bool operator==(const FFlecsEntityRecord& Other) const
	{
		return Components == Other.Components && SubEntities == Other.SubEntities && Fragments == Other.Fragments;
	}

	NO_DISCARD FORCEINLINE bool operator!=(const FFlecsEntityRecord& Other) const
	{
		return !(*this == Other);
	}
	
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
	
	FORCEINLINE FFlecsEntityRecord& AddComponent(const FInstancedStruct& InStruct)
	{
		FFlecsComponentTypeInfo NewComponent;
		NewComponent.NodeType = EFlecsComponentNodeType::ScriptStruct;
		NewComponent.ScriptStruct = InStruct;
		Components.Add(NewComponent);

		return *this;
	}
	
	FORCEINLINE FFlecsEntityRecord& AddComponent(FInstancedStruct&& InStruct)
	{
		FFlecsComponentTypeInfo NewComponent;
		NewComponent.NodeType = EFlecsComponentNodeType::ScriptStruct;
		NewComponent.ScriptStruct = MoveTemp(InStruct);
		Components.Add(NewComponent);

		return *this;
	}

	template <Unreal::Flecs::CNonStructUtilScriptStructType T>
	requires (!std::is_enum_v<T>)
	FORCEINLINE FFlecsEntityRecord& AddComponent()
	{
		FFlecsComponentTypeInfo NewComponent;
		NewComponent.NodeType = EFlecsComponentNodeType::ScriptStruct;
		NewComponent.ScriptStruct = FInstancedStruct::Make<T>();
		Components.Add(NewComponent);

		return *this;
	}

	template <Unreal::Flecs::CNonStructUtilScriptStructType T>
	requires (!std::is_enum_v<T>)
	FORCEINLINE FFlecsEntityRecord& AddComponent(const T& InComponent)
	{
		FFlecsComponentTypeInfo NewComponent;
		NewComponent.NodeType = EFlecsComponentNodeType::ScriptStruct;
		NewComponent.ScriptStruct = FInstancedStruct::Make<T>(InComponent);
		Components.Add(NewComponent);

		return *this;
	}

	template <Unreal::Flecs::CNonStructUtilScriptStructType T>
	requires (!std::is_enum_v<T>)
	FORCEINLINE FFlecsEntityRecord& AddComponent(T&& InComponent)
	{
		FFlecsComponentTypeInfo NewComponent;
		NewComponent.NodeType = EFlecsComponentNodeType::ScriptStruct;
		NewComponent.ScriptStruct = FInstancedStruct::Make<T>(MoveTemp(InComponent));
		Components.Add(NewComponent);

		return *this;
	}

	FORCEINLINE FFlecsEntityRecord& AddComponent(const FFlecsId InEntityHandle)
	{
		FFlecsComponentTypeInfo NewComponent;
		NewComponent.NodeType = EFlecsComponentNodeType::EntityHandle;
		NewComponent.EntityHandle = InEntityHandle;
		Components.Add(NewComponent);

		return *this;
	}

	FORCEINLINE FFlecsEntityRecord& AddComponent(const FGameplayTag& InGameplayTag)
	{
		FFlecsComponentTypeInfo NewComponent;
		NewComponent.NodeType = EFlecsComponentNodeType::FGameplayTag;
		NewComponent.GameplayTag = InGameplayTag;
		Components.Add(NewComponent);

		return *this;
	}

	FORCEINLINE FFlecsEntityRecord& AddComponent(FGameplayTag&& InGameplayTag)
	{
		FFlecsComponentTypeInfo NewComponent;
		NewComponent.NodeType = EFlecsComponentNodeType::FGameplayTag;
		NewComponent.GameplayTag = MoveTemp(InGameplayTag);
		Components.Add(NewComponent);

		return *this;
	}

	FORCEINLINE FFlecsEntityRecord& AddComponent(const FSolidEnumSelector& InScriptEnum)
	{
		FFlecsComponentTypeInfo NewComponent;
		NewComponent.NodeType = EFlecsComponentNodeType::ScriptEnum;
		NewComponent.ScriptEnum = InScriptEnum;
		Components.Add(NewComponent);

		return *this;
	}

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

	FORCEINLINE FFlecsEntityRecord& AddComponent(const FFlecsRecordPair& InPair)
	{
		FFlecsComponentTypeInfo NewComponent;
		NewComponent.NodeType = EFlecsComponentNodeType::Pair;
		NewComponent.Pair = InPair;
		Components.Add(NewComponent);

		return *this;
	}

	FORCEINLINE FFlecsEntityRecord& AddComponent(FFlecsRecordPair&& InPair)
	{
		FFlecsComponentTypeInfo NewComponent;
		NewComponent.NodeType = EFlecsComponentNodeType::Pair;
		NewComponent.Pair = MoveTemp(InPair);
		Components.Add(NewComponent);

		return *this;
	}

	FORCEINLINE int32 AddSubEntity(const FFlecsEntityRecord& InSubEntity, const bool bInDontFragmentParentChildRelationship = true)
	{
		return SubEntities.Add(FFlecsSubEntityRecord(
			TInstancedStruct<FFlecsEntityRecord>::Make(InSubEntity),
			bInDontFragmentParentChildRelationship));
	}

	FORCEINLINE void RemoveSubEntity(const int32 InIndex)
	{
		solid_checkf(SubEntities.IsValidIndex(InIndex), TEXT("Index is out of bounds"));
		SubEntities.RemoveAt(InIndex);
	}

	FORCEINLINE void RemoveAllSubEntities()
	{
		SubEntities.Empty();
	}

	NO_DISCARD FORCEINLINE bool HasSubEntities() const
	{
		return !SubEntities.IsEmpty();
	}

	NO_DISCARD FORCEINLINE int32 GetSubEntityCount() const
	{
		return SubEntities.Num();
	}

	NO_DISCARD FORCEINLINE TConstStructView<FFlecsEntityRecord> GetSubEntity(const int32 InIndex) const
	{
		solid_checkf(SubEntities.IsValidIndex(InIndex), TEXT("Index is out of bounds"));
		return SubEntities[InIndex].Record;
	}

	NO_DISCARD FORCEINLINE TStructView<FFlecsEntityRecord> GetSubEntity(const int32 InIndex)
	{
		solid_checkf(SubEntities.IsValidIndex(InIndex), TEXT("Index is out of bounds"));
		return SubEntities[InIndex].Record;
	}

	template <Unreal::Flecs::CNonStructUtilScriptStructType TFragmentType>
	requires (std::is_base_of_v<FFlecsEntityRecordFragment, TFragmentType>)
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

	template <Unreal::Flecs::CNonStructUtilScriptStructType TFragmentType, typename... TArgs>
	requires (std::is_base_of_v<FFlecsEntityRecordFragment, TFragmentType>)
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

	template <Unreal::Flecs::CNonStructUtilScriptStructType TFragmentType>
	requires (std::is_base_of_v<FFlecsEntityRecordFragment, TFragmentType>)
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
	NO_DISCARD FORCEINLINE bool HasFragment() const
	{
		return HasFragment(TBaseStructure<TFragmentType>::Get());
	}
	
	NO_DISCARD FORCEINLINE bool HasFragments() const
	{
		return !Fragments.IsEmpty();
	}

	NO_DISCARD FORCEINLINE int32 GetFragmentCount() const
	{
		return Fragments.Num();
	}

	NO_DISCARD FORCEINLINE TConstStructView<FFlecsEntityRecordFragment> GetFragment(const int32 InIndex) const
	{
		solid_checkf(Fragments.IsValidIndex(InIndex), TEXT("Index is out of bounds"));
		return Fragments[InIndex];
	}

	NO_DISCARD FORCEINLINE TStructView<FFlecsEntityRecordFragment> GetFragment(const int32 InIndex)
	{
		solid_checkf(Fragments.IsValidIndex(InIndex), TEXT("Index is out of bounds"));
		return Fragments[InIndex];
	}

	void ApplyRecordToEntity(const TSolidNotNull<const UFlecsWorld*> InFlecsWorld, const FFlecsEntityHandle& InEntityHandle) const;

}; // struct FFlecsEntityRecord

REGISTER_FLECS_COMPONENT(FFlecsEntityRecord,
	[](flecs::world InWorld, const FFlecsComponentHandle& InComponentHandle)
	{
		InComponentHandle
			.AddPair(flecs::OnInstantiate, flecs::DontInherit);
	});

// @TODO: add additional settings
USTRUCT(BlueprintType, DisplayName = "Named Entity Fragment")
struct UNREALFLECS_API FFlecsNamedEntityRecordFragment : public FFlecsEntityRecordFragment
{
	GENERATED_BODY()

public:
	FORCEINLINE FFlecsNamedEntityRecordFragment() = default;
	FORCEINLINE FFlecsNamedEntityRecordFragment(const FString& InName)
		: Name(InName)
	{
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity Record Fragment")
	FString Name;
	
	virtual void PreApplyRecordToEntity(
			const TSolidNotNull<const UFlecsWorld*> InFlecsWorld, const FFlecsEntityHandle& InEntityHandle) const override;
	
	struct FBuilder;
	
}; // struct FFlecsNamedEntityRecordFragment

struct FFlecsNamedEntityRecordFragment::FBuilder : public FFlecsEntityRecord::FFragmentBuilderType<FFlecsNamedEntityRecordFragment>
{
	using Super = FFlecsEntityRecord::FFragmentBuilderType<FFlecsNamedEntityRecordFragment>;
	using Super::Super;
	
public:
	FORCEINLINE FBuilder& Named(const FString& InName)
	{
		this->GetSelf().Name = InName;
		return *this;
	}
		
}; // struct FFlecsNamedEntityRecordFragment::FBuilder

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsEntityRecordComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity Record")
	FFlecsEntityRecord EntityRecord;
	
}; // struct FFlecsEntityRecordComponent

REGISTER_FLECS_COMPONENT(FFlecsEntityRecordComponent,
	[](flecs::world InWorld, const FFlecsComponentHandle& InComponentHandle)
	{
		InComponentHandle
			.AddPair(flecs::OnInstantiate, flecs::DontInherit);
	});

	