// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Entities/FlecsEntityHandle.h"
#include "SolidMacros/Macros.h"
#include "SolidMacros/Concepts/SolidConcepts.h"
#include "Types/SolidNonNullPtr.h"
#include "FlecsCollectionBuilder.generated.h"

struct FFlecsComponentCollection;

USTRUCT()
struct UNREALFLECS_API FFlecsCollectionItem
{
	GENERATED_BODY()

	NO_DISCARD FORCEINLINE friend uint32 GetTypeHash(const FFlecsCollectionItem& InItem)
	{
		if (InItem.Second.IsEmpty())
		{
			return GetTypeHash(InItem.First);
		}
		else
		{
			return HashCombineFast(GetTypeHash(InItem.First), GetTypeHash(InItem.Second));
		}
	}

public:
	UPROPERTY()
	FString First;

	UPROPERTY()
	FString Second;

	UPROPERTY()
	TArray<uint8> ComponentData;
	
}; // struct FFlecsCollectionItem

USTRUCT()
struct UNREALFLECS_API FFlecsCollectionBuilder
{
	GENERATED_BODY()

	friend class UFlecsComponentCollectionObject;

public:
	FFlecsCollectionBuilder(const bool bInIsSlotEntity = false)
		: bIsSlotEntity(bInIsSlotEntity)
	{
	}

	void ApplyToEntity(const FFlecsEntityHandle& InEntityHandle, TSolidNonNullPtr<UFlecsWorld> InWorld) const;
	NO_DISCARD bool ValidateData(FFlecsComponentCollection& InComponentCollection) const;

	template <typename T>
	FORCEINLINE void AddComponent(const T& InComponent)
	{
		FFlecsCollectionItem NewItem;
		NewItem.First = GetFirst<T>();
		
		NewItem.ComponentData = TArray(reinterpret_cast<const uint8*>(&InComponent),
			sizeof(T));
		
		CollectionItems.Add(NewItem);
	}

	template <typename T>
	FORCEINLINE void AddComponent()
	{
		FFlecsCollectionItem NewItem;
		NewItem.First = GetFirst<T>();
		CollectionItems.Add(NewItem);
	}

	FORCEINLINE void AddComponent(const TSolidNonNullPtr<UScriptStruct> StructType, const void* InComponent = nullptr)
	{
		solid_check(IsValid(StructType));
		
		FFlecsCollectionItem NewItem;
		NewItem.First = GetFirst(StructType);
		
		if (InComponent)
		{
			NewItem.ComponentData = TArray<uint8>(reinterpret_cast<const uint8*>(InComponent),
				StructType->GetStructureSize());
		}
		
		CollectionItems.Add(NewItem);
	}

	template <typename T>
	NO_DISCARD FORCEINLINE T& GetComponent()
	{
		const FFlecsCollectionItem* Item = CollectionItems.FindByPredicate([&](const FFlecsCollectionItem& InItem)
		{
			return InItem.First == GetFirst<T>();
		});

		solid_check(Item);
		return *reinterpret_cast<T*>(Item->ComponentData.GetData());
	}

	template <typename T>
	NO_DISCARD FORCEINLINE const T& GetComponent() const
	{
		const FFlecsCollectionItem* Item = CollectionItems.FindByPredicate([&](const FFlecsCollectionItem& InItem)
		{
			return InItem.First == GetFirst<T>();
		});

		solid_check(Item);
		return *reinterpret_cast<const T*>(Item->ComponentData.GetData());
	}

	NO_DISCARD FORCEINLINE void* GetComponent(const FString& InFirst) const
	{
		return (void*)CollectionItems.FindByPredicate([&](const FFlecsCollectionItem& Item)
		{
			return Item.First == InFirst;
		})->ComponentData.GetData();
	}

	NO_DISCARD FORCEINLINE void* GetComponent(const TSolidNonNullPtr<UScriptStruct> StructType) const
	{
		solid_check(IsValid(StructType));
		return GetComponent(GetFirst(StructType));
	}

	NO_DISCARD FORCEINLINE bool HasComponent(const FString& InFirst) const
	{
		return CollectionItems.ContainsByPredicate([&](const FFlecsCollectionItem& Item)
		{
			return Item.First == InFirst;
		});
	}

	template <typename T>
	NO_DISCARD FORCEINLINE bool HasComponent() const
	{
		return HasComponent(GetFirst<T>());
	}

	NO_DISCARD FORCEINLINE bool HasComponent(const TSolidNonNullPtr<UScriptStruct> StructType) const
	{
		solid_check(IsValid(StructType));
		return HasComponent(GetFirst(StructType));
	}

	FORCEINLINE void Clear()
	{
		CollectionItems.Empty();
	}

	FORCEINLINE void RemoveComponent(const FString& InFirst)
	{
		CollectionItems.RemoveAll([&](const FFlecsCollectionItem& Item)
		{
			return Item.First == InFirst;
		});
	}

	FORCEINLINE void RemoveComponent(const TSolidNonNullPtr<UScriptStruct> StructType)
	{
		solid_check(IsValid(StructType));
		RemoveComponent(GetFirst(StructType));
	}

	template <typename T>
	FORCEINLINE void RemoveComponent()
	{
		RemoveComponent(GetFirst<T>());
	}

	FORCEINLINE void AddComponentRequirement(const FString& InFirst)
	{
		ComponentRequirements.Add(InFirst);
	}

	FORCEINLINE void AddComponentExclusion(const FString& InFirst)
	{
		ComponentExclusions.Add(InFirst);
	}

	FORCEINLINE void ClearComponentRequirements()
	{
		ComponentRequirements.Empty();
	}

	FORCEINLINE void ClearComponentExclusions()
	{
		ComponentExclusions.Empty();
	}

	FORCEINLINE void ClearComponentRequirementsAndExclusions()
	{
		ClearComponentRequirements();
		ClearComponentExclusions();
	}

	FORCEINLINE void ClearAll()
	{
		Clear();
		ClearComponentRequirementsAndExclusions();
	}

	FORCEINLINE void RemoveComponentRequirement(const FString& InFirst)
	{
		ComponentRequirements.Remove(InFirst);
	}

	FORCEINLINE void RemoveComponentExclusion(const FString& InFirst)
	{
		ComponentExclusions.Remove(InFirst);
	}

private:
	UPROPERTY()
	TArray<FFlecsCollectionItem> CollectionItems;

	UPROPERTY()
	TSet<FString> ComponentRequirements;

	UPROPERTY()
	TSet<FString> ComponentExclusions;

	UPROPERTY()
	bool bIsSlotEntity;
	
	template <typename T>
	NO_DISCARD FORCEINLINE FString GetFirst() const
	{
		return nameof(T);
	}

	NO_DISCARD FORCEINLINE FString GetFirst(const UScriptStruct* StructType) const
	{
		solid_check(IsValid(StructType));
		return StructType->GetStructCPPName();
	}

	template <Solid::TBaseStructureConcept T>
	NO_DISCARD FORCEINLINE FString GetFirst() const
	{
		return GetFirst(TBaseStructure<T>::Get());
	}
	
}; // struct FFlecsCollectionBuilder

