﻿// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Collections/FlecsCollectionItemBuilder.h"
#include "Collections/FlecsComponentCollectionObject.h"
#include "StructUtils/InstancedStruct.h"
#include "Entities/FlecsEntityHandle.h"
#include "Properties/FlecsComponentProperties.h"
#include "Types/SolidEnumSelector.h"
#include "FlecsEntityRecord.generated.h"

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

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsRecordPairSlot
{
	GENERATED_BODY()

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
	First = 0,
}; // enum class EFlecsValuePairType

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsRecordPair
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component Tree")
	FFlecsRecordPairSlot First;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component Tree")
	FFlecsRecordPairSlot Second;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component Tree")
	EFlecsValuePairType PairValueType = EFlecsValuePairType::First;

	NO_DISCARD FORCEINLINE bool operator==(const FFlecsRecordPair& Other) const
	{
		return First == Other.First && Second == Other.Second;
	}

	NO_DISCARD FORCEINLINE bool operator!=(const FFlecsRecordPair& Other) const
	{
		return !(*this == Other);
	}

	FORCEINLINE void AddToEntity(const FFlecsEntityHandle& InEntityHandle) const
	{
		switch (First.PairNodeType)
		{
		case EFlecsPairNodeType::ScriptStruct:
			{
				switch (Second.PairNodeType)
				{
					case EFlecsPairNodeType::ScriptStruct:
						{
							if (PairValueType == EFlecsValuePairType::First)
							{
								InEntityHandle.SetPair(First.PairScriptStruct.GetScriptStruct(),
									First.PairScriptStruct.GetMemory(), Second.PairScriptStruct.GetScriptStruct());
							}
						}
					break;
					case EFlecsPairNodeType::EntityHandle:
						{
							InEntityHandle.AddPair(First.PairScriptStruct.GetScriptStruct(), Second.EntityHandle);
						}
					break;
					case EFlecsPairNodeType::FGameplayTag:
						{
							InEntityHandle.AddPair(First.PairScriptStruct.GetScriptStruct(), Second.GameplayTag);
						}
					break;
				}
			}
			break;
		case EFlecsPairNodeType::EntityHandle:
			{
				switch (Second.PairNodeType)
				{
					case EFlecsPairNodeType::ScriptStruct:
						{
							InEntityHandle.AddPair(First.EntityHandle, Second.PairScriptStruct.GetScriptStruct());
							InEntityHandle.SetPairSecond(First.EntityHandle, Second.PairScriptStruct.GetScriptStruct(),
								Second.PairScriptStruct.GetMemory());
						}
					break;
					case EFlecsPairNodeType::EntityHandle:
						{
							InEntityHandle.AddPair(First.EntityHandle, Second.EntityHandle);
						}
					break;
					case EFlecsPairNodeType::FGameplayTag:
						{
							InEntityHandle.AddPair(First.EntityHandle, Second.GameplayTag);
						}
					break;
				}
			}
			break;
		case EFlecsPairNodeType::FGameplayTag:
			{
				switch (Second.PairNodeType)
				{
					case EFlecsPairNodeType::ScriptStruct:
						{
							InEntityHandle.AddPair(First.GameplayTag, Second.PairScriptStruct.GetScriptStruct());
							InEntityHandle.SetPairSecond(First.GameplayTag, Second.PairScriptStruct.GetScriptStruct(),
								Second.PairScriptStruct.GetMemory());
						}
					break;
					case EFlecsPairNodeType::EntityHandle:
						{
							InEntityHandle.AddPair(First.GameplayTag, Second.EntityHandle);
						}
					break;
					case EFlecsPairNodeType::FGameplayTag:
						{
							InEntityHandle.AddPair(First.GameplayTag, Second.GameplayTag);
						}
					break;
				}
			}
			break;
		}
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

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsRecordSubEntity
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component Tree")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity Record")
	TArray<FFlecsComponentTypeInfo> Components;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "Entity Record")
	TArray<TObjectPtr<UFlecsComponentCollectionObject>> Collections;

	NO_DISCARD FORCEINLINE bool operator==(const FFlecsRecordSubEntity& Other) const
	{
		return Name == Other.Name && Components == Other.Components;
	}

	NO_DISCARD FORCEINLINE bool operator!=(const FFlecsRecordSubEntity& Other) const
	{
		return !(*this == Other);
	}

	FORCEINLINE void ApplyRecordToEntity(const FFlecsEntityHandle& InEntityHandle) const
	{
		solid_checkf(InEntityHandle.IsValid(), TEXT("Entity Handle is not valid"));

		if (!Name.IsEmpty() && !InEntityHandle.HasName())
		{
			InEntityHandle.SetName(Name);
		}

		for (const auto& [NodeType, ScriptStruct, ScriptEnum, EntityHandle, GameplayTag, Pair] : Components)
		{
			switch (NodeType)
			{
			case EFlecsComponentNodeType::ScriptStruct:
				{
					InEntityHandle.Set(ScriptStruct);
				}
				break;
			case EFlecsComponentNodeType::ScriptEnum:
				{
					InEntityHandle.Add(ScriptEnum.Class, ScriptEnum.Value);
				}
			case EFlecsComponentNodeType::EntityHandle:
				{
					InEntityHandle.Add(EntityHandle);
				}
				break;
			case EFlecsComponentNodeType::FGameplayTag:
				{
					InEntityHandle.Add(GameplayTag);	
				}
				break;
			case EFlecsComponentNodeType::Pair:
				{
					Pair.AddToEntity(InEntityHandle);
				}
				break;
			}
		}

		for (UFlecsComponentCollectionObject* Collection : Collections)
		{
			InEntityHandle.AddCollection(Collection);
		}
	}
	
}; // struct FFlecsRecordSubEntity


/**
 * @brief A record of a generic entity's components and sub-entities,
 * this can be applied to an actual entity to give it the same components/sub-entities.
 */
USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsEntityRecord
{
	GENERATED_BODY()

	FORCEINLINE FFlecsEntityRecord() = default;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity Record")
	TArray<FFlecsComponentTypeInfo> Components;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "Entity Record")
	TArray<TObjectPtr<UFlecsComponentCollectionObject>> Collections;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity Record")
	TArray<FFlecsRecordSubEntity> SubEntities;

	UPROPERTY(BlueprintReadWrite, Category = "Entity Record")
	FFlecsCollectionItemBuilder CollectionBuilder;

	NO_DISCARD FORCEINLINE bool operator==(const FFlecsEntityRecord& Other) const
	{
		return Components == Other.Components;
	}

	NO_DISCARD FORCEINLINE bool operator!=(const FFlecsEntityRecord& Other) const
	{
		return !(*this == Other);
	}

	template <Solid::TScriptStructConcept T>
	FORCEINLINE void AddComponent(const T& InComponent)
	{
		FFlecsComponentTypeInfo NewComponent;
		NewComponent.NodeType = EFlecsComponentNodeType::ScriptStruct;
		NewComponent.ScriptStruct = FInstancedStruct::Make<T>(InComponent);
		Components.Add(NewComponent);
	}

	FORCEINLINE void AddComponent(const FFlecsEntityHandle& InEntityHandle)
	{
		FFlecsComponentTypeInfo NewComponent;
		NewComponent.NodeType = EFlecsComponentNodeType::EntityHandle;
		NewComponent.EntityHandle = InEntityHandle;
		Components.Add(NewComponent);
	}

	FORCEINLINE void AddComponent(const FGameplayTag& InGameplayTag)
	{
		FFlecsComponentTypeInfo NewComponent;
		NewComponent.NodeType = EFlecsComponentNodeType::FGameplayTag;
		NewComponent.GameplayTag = InGameplayTag;
		Components.Add(NewComponent);
	}

	FORCEINLINE void AddComponent(const FFlecsRecordPair& InPair)
	{
		FFlecsComponentTypeInfo NewComponent;
		NewComponent.NodeType = EFlecsComponentNodeType::Pair;
		NewComponent.Pair = InPair;
		Components.Add(NewComponent);
	}

	FORCEINLINE int32 AddSubEntity(const FFlecsRecordSubEntity& InSubEntity)
	{
		return SubEntities.Add(InSubEntity);
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

	FORCEINLINE const FFlecsRecordSubEntity& GetSubEntity(const int32 InIndex) const
	{
		solid_checkf(SubEntities.IsValidIndex(InIndex), TEXT("Index is out of bounds"));
		return SubEntities[InIndex];
	}

	FORCEINLINE FFlecsRecordSubEntity& GetSubEntity(const int32 InIndex)
	{
		solid_checkf(SubEntities.IsValidIndex(InIndex), TEXT("Index is out of bounds"));
		return SubEntities[InIndex];
	}

	FORCEINLINE void ApplyRecordToEntity(const FFlecsEntityHandle& InEntityHandle) const
	{
		solid_checkf(InEntityHandle.IsValid(), TEXT("Entity Handle is not valid"));

		for (const auto& [NodeType, ScriptStruct, ScriptEnum, EntityHandle, GameplayTag, Pair] : Components)
		{
			switch (NodeType)
			{
			case EFlecsComponentNodeType::ScriptStruct:
				{
					InEntityHandle.Set(ScriptStruct);
				}
				break;
			case EFlecsComponentNodeType::ScriptEnum:
				{
					InEntityHandle.Add(ScriptEnum.Class, ScriptEnum.Value);
				}
			case EFlecsComponentNodeType::EntityHandle:
				{
					InEntityHandle.Add(EntityHandle);
				}
				break;
			case EFlecsComponentNodeType::FGameplayTag:
				{
					InEntityHandle.Add(GameplayTag);	
				}
				break;
			case EFlecsComponentNodeType::Pair:
				{
					Pair.AddToEntity(InEntityHandle);
				}
				break;
			}
		}

		for (UFlecsComponentCollectionObject* Collection : Collections)
		{
			InEntityHandle.AddCollection(Collection);
		}

		for (const FFlecsRecordSubEntity& SubEntity : SubEntities)
		{
			FFlecsEntityHandle NewEntityHandle = InEntityHandle
				.GetEntity().world().entity(StringCast<char>(*SubEntity.Name).Get());
			NewEntityHandle.SetParent(InEntityHandle);
			
			SubEntity.ApplyRecordToEntity(NewEntityHandle);
			InEntityHandle.Add(NewEntityHandle);
		}
	}
	
}; // struct FFlecsEntityRecord

REGISTER_FLECS_COMPONENT(FFlecsEntityRecord, [](flecs::world InWorld, flecs::untyped_component InComponent)
	{
		InComponent.add(flecs::OnInstantiate, flecs::DontInherit);
	});
