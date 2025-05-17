// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Entities/FlecsEntityHandle.h"
#include "SolidMacros/Macros.h"
#include "SolidMacros/Concepts/SolidConcepts.h"
#include "FlecsCollectionItemBuilder.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsCollectionItem
{
	GENERATED_BODY()

	NO_DISCARD FORCEINLINE friend uint32 GetTypeHash(const FFlecsCollectionItem& InItem)
	{
		return GetTypeHash(InItem.ComponentName);
	}

public:
	UPROPERTY()
	FString ComponentName;

	UPROPERTY()
	TArray<uint8> ComponentData;
}; // struct FFlecsCollectionItem

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsCollectionItemBuilder
{
	GENERATED_BODY()

	friend class UFlecsComponentCollectionObject;

public:
	FFlecsCollectionItemBuilder()
	{
	}

	void ApplyToEntity(const FFlecsEntityHandle& InEntityHandle, UFlecsWorld* InWorld) const;

	template <typename T>
	FORCEINLINE void AddComponent(const T& InComponent)
	{
		FFlecsCollectionItem NewItem;
		NewItem.ComponentName = GetComponentName<T>();
		NewItem.ComponentData = TArray<uint8>(reinterpret_cast<const uint8*>(&InComponent),
			sizeof(T));
		CollectionItems.Add(NewItem);
	}

	template <typename T>
	FORCEINLINE void AddComponent()
	{
		FFlecsCollectionItem NewItem;
		NewItem.ComponentName = GetComponentName<T>();
		CollectionItems.Add(NewItem);
	}

	FORCEINLINE void AddComponent(const UScriptStruct* StructType, const void* InComponent = nullptr)
	{
		solid_check(IsValid(StructType));
		FFlecsCollectionItem NewItem;
		NewItem.ComponentName = GetComponentName(StructType);
		
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
			return InItem.ComponentName == GetComponentName<T>();
		});

		solid_check(Item);
		return *reinterpret_cast<T*>(Item->ComponentData.GetData());
	}

	template <typename T>
	NO_DISCARD FORCEINLINE const T& GetComponent() const
	{
		const FFlecsCollectionItem* Item = CollectionItems.FindByPredicate([&](const FFlecsCollectionItem& InItem)
		{
			return InItem.ComponentName == GetComponentName<T>();
		});

		solid_check(Item);
		return *reinterpret_cast<const T*>(Item->ComponentData.GetData());
	}

	NO_DISCARD FORCEINLINE void* GetComponent(const FString& InComponentName) const
	{
		return (void*)CollectionItems.FindByPredicate([&](const FFlecsCollectionItem& Item)
		{
			return Item.ComponentName == InComponentName;
		})->ComponentData.GetData();
	}

	NO_DISCARD FORCEINLINE void* GetComponent(const UScriptStruct* StructType) const
	{
		return GetComponent(GetComponentName(StructType));
	}

	NO_DISCARD FORCEINLINE bool HasComponent(const FString& InComponentName) const
	{
		return CollectionItems.ContainsByPredicate([&](const FFlecsCollectionItem& Item)
		{
			return Item.ComponentName == InComponentName;
		});
	}

	template <typename T>
	NO_DISCARD FORCEINLINE bool HasComponent() const
	{
		return HasComponent(GetComponentName<T>());
	}

	NO_DISCARD FORCEINLINE bool HasComponent(const UScriptStruct* StructType) const
	{
		return HasComponent(GetComponentName(StructType));
	}

	FORCEINLINE void Clear()
	{
		CollectionItems.Empty();
	}

	FORCEINLINE void RemoveComponent(const FString& InComponentName)
	{
		CollectionItems.RemoveAll([&](const FFlecsCollectionItem& Item)
		{
			return Item.ComponentName == InComponentName;
		});
	}

	FORCEINLINE void RemoveComponent(const UScriptStruct* StructType)
	{
		RemoveComponent(GetComponentName(StructType));
	}

	template <typename T>
	FORCEINLINE void RemoveComponent()
	{
		RemoveComponent(GetComponentName<T>());
	}

	FORCEINLINE void AddComponentRequirement(const FString& InComponentName)
	{
		ComponentRequirements.Add(InComponentName);
	}

	FORCEINLINE void AddComponentExclusion(const FString& InComponentName)
	{
		ComponentExclusions.Add(InComponentName);
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

	FORCEINLINE void RemoveComponentRequirement(const FString& InComponentName)
	{
		ComponentRequirements.Remove(InComponentName);
	}

	FORCEINLINE void RemoveComponentExclusion(const FString& InComponentName)
	{
		ComponentExclusions.Remove(InComponentName);
	}

	FORCEINLINE void SetNewEntity(const bool bInNewEntity)
	{
		bNewEntity = bInNewEntity;
	}

	NO_DISCARD FORCEINLINE bool IsNewEntity() const
	{
		return bNewEntity;
	}
	
private:
	UPROPERTY()
	TArray<FFlecsCollectionItem> CollectionItems;

	UPROPERTY()
	TSet<FString> ComponentRequirements;

	UPROPERTY()
	TSet<FString> ComponentExclusions;

	UPROPERTY()
	bool bNewEntity = false;

	template <typename T>
	NO_DISCARD FORCEINLINE FString GetComponentName() const
	{
		return nameof(T);
	}

	NO_DISCARD FORCEINLINE FString GetComponentName(const UScriptStruct* StructType) const
	{
		solid_check(IsValid(StructType));
		return StructType->GetStructCPPName();
	}

	template <Solid::TBaseStructureConcept T>
	NO_DISCARD FORCEINLINE FString GetComponentName() const
	{
		return GetComponentName(TBaseStructure<T>::Get());
	}
	
}; // struct FFlecsCollectionItemBuilder

