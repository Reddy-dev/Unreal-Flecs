﻿// Solstice Games © 2024. All Rights Reserved.

// ReSharper disable CppMemberFunctionMayBeStatic
// ReSharper disable CppExpressionWithoutSideEffects
// ReSharper disable CppUnusedIncludeDirective
#pragma once

#include <unordered_map>

#include "CoreMinimal.h"
#include "flecs.h"
#include "FlecsWorld.h"
#include "FlecsWorldDeveloperSettings.h"
#include "FlecsWorldSettings.h"
#include "GameplayTagContainer.h"
#include "GameplayTagsManager.h"
#include "Components/FlecsGameplayTagEntityComponent.h"
#include "Components/FlecsWorldPtrComponent.h"
#include "Entities/FlecsDefaultEntityEngineSubsystem.h"
#include "General/FlecsDeveloperSettings.h"
#include "SolidMacros/Concepts/SolidConcepts.h"
#include "SolidMacros/Standard/Hashing.h"
#include "Standard/robin_hood.h"
#include "Subsystems/WorldSubsystem.h"
#include "FlecsWorldSubsystem.generated.h"

// ReSharper disable once CppUE4CodingStandardNamingViolationWarning
constexpr std::string_view DEFAULT_FLECS_WORLD_NAME = "DefaultFlecsWorld";

DEFINE_STD_HASH(FString);

USTRUCT(BlueprintType)
struct FFlecsRestSettings
{
	GENERATED_BODY()

	/* Defaults Set, according to flecs's defaults */
	
	UPROPERTY(EditAnywhere, Category = "Flecs | REST API",
		meta = (ClampMin = "0", ClampMax = "65535", UIMin = "0", UIMax = "65535"))
	int32 Port = 27750;

	UPROPERTY(EditAnywhere, Category = "Flecs | REST API")
	FString IPAddress = "0.0.0.0";
	
}; // struct FFlecsRestSettings

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnWorldCreated, const FString&, UFlecsWorld*);

UCLASS(BlueprintType)
class UNREALFLECS_API UFlecsWorldSubsystem final : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override
	{
		return GetDefault<UFlecsDeveloperSettings>() && GetDefault<UFlecsDeveloperSettings>()->bEnableFlecs;
	}

	virtual ETickableTickType GetTickableTickType() const override
	{
		return GetDefault<UFlecsDeveloperSettings>()
			&& GetDefault<UFlecsDeveloperSettings>()->bAutoTickWorld ? ETickableTickType::Always : ETickableTickType::Never;
	}
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override
	{
		Super::Initialize(Collection);
		
		DeveloperSettings = GetDefault<UFlecsDeveloperSettings>();
	}

	virtual void OnWorldBeginPlay(UWorld& InWorld) override
	{
		UFlecsWorldDeveloperSettings* WorldDeveloperSettings = GetMutableDefault<UFlecsWorldDeveloperSettings>();
		
		for (const FFlecsWorldSettings& Settings : WorldDeveloperSettings->Worlds)
		{
			CreateWorld(Settings.WorldName, Settings);
		}
	}

	virtual void Deinitialize() override
	{
		for (UFlecsWorld* World : Worlds)
		{
			World->DestroyWorld();
		}
		
		Super::Deinitialize();
	}

	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(UFlecsWorldSubsystem, STATGROUP_Tickables);
	}

	virtual void Tick(float DeltaTime) override
	{
		for (const UFlecsWorld* World : Worlds)
		{
			World->Progress();
		}
	}
	
	UFUNCTION(BlueprintCallable, Category = "Flecs")
	FORCEINLINE UFlecsWorld* CreateWorld(const FString& Name, const FFlecsWorldSettings& Settings)
	{
		solid_checkf(!HasWorld(Name), TEXT("World with name %s already exists"), *Name);
		solid_checkf(!Name.IsEmpty(), TEXT("World name cannot be NAME_None"));

		flecs::world NewWorld = flecs::world();

		TArray<FFlecsDefaultMetaEntity> DefaultEntities = GetDefault<UFlecsDefaultEntityEngineSubsystem>()->AddedDefaultEntities;

		UFlecsWorld* NewFlecsWorld = NewObject<UFlecsWorld>(this);
		NewFlecsWorld->SetWorld(std::move(NewWorld));
		
		Worlds.Add(NewFlecsWorld);

		NewFlecsWorld->SetName(Name);
		
		NewFlecsWorld->SetContext(this);

		NewFlecsWorld->SetSingleton<FFlecsWorldPtrComponent>(FFlecsWorldPtrComponent
			{ NewFlecsWorld, GetWorld() });
		
		WorldNameMap.emplace(Name, NewFlecsWorld);

		for (const auto& [EntityRecord, bIsOptionEntity] : DefaultEntities)
		{
			NewFlecsWorld->CreateEntityWithRecord(EntityRecord);
		}

		#if WITH_EDITOR || UE_BUILD_DEBUG
		
		if (DeveloperSettings->bAutoImportExplorer)
		{
			ImportMonitoringModule(Name);
		}
		
		#endif // WITH_EDITOR

		GetDefaultWorld(this)->AddSingleton<FFlecsTypeMapComponent>();

		NewFlecsWorld->SetThreads(DeveloperSettings->DefaultWorkerThreads);

		RegisterAllGameplayTags(NewFlecsWorld);

		for (UObject* Module : Settings.Modules)
		{
			solid_checkf(Module->GetClass()->ImplementsInterface(UFlecsModuleInterface::StaticClass()),
				TEXT("Module %s does not implement UFlecsModuleInterface"), *Module->GetName());
			
			NewFlecsWorld->ImportModule(Module);
		}
		
		NewFlecsWorld->WorldBeginPlay();
		
		OnWorldCreated.Broadcast(Name, NewFlecsWorld);
		
		return Worlds.Last();
	}

	UFUNCTION(BlueprintCallable, Category = "Flecs")
	FORCEINLINE bool HasWorld(const FString& Name) const
	{
		return WorldNameMap.contains(Name);
	}

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs | REST API")
	FORCEINLINE void ImportRestModule(const FString& WorldName, const bool bUseMonitoring, const FFlecsRestSettings& Settings) const
	{
		GetFlecsWorld(WorldName)->SetSingleton<flecs::Rest>(
				flecs::Rest { static_cast<uint16>(Settings.Port),
			const_cast<ANSICHAR*>(StringCast<ANSICHAR>(*Settings.IPAddress).Get()) });

		if (bUseMonitoring)
		{
			ImportMonitoringModule(WorldName);
		}
	}

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs")
	FORCEINLINE void ImportMonitoringModule(const FString& WorldName) const
	{
		GetFlecsWorld(WorldName)->ImportModule<flecs::monitor>();
	}

	UFUNCTION(BlueprintCallable, Category = "Flecs")
	FORCEINLINE void DestroyWorldByName(const FString& Name)
	{
		DestroyWorld(GetFlecsWorld(Name));
	}

	UFUNCTION(BlueprintCallable, Category = "Flecs")
	FORCEINLINE void DestroyWorld(UFlecsWorld* World)
	{
		WorldNameMap.erase(World->GetWorldName());

		for (int32 Index = 0; Index < Worlds.Num(); ++Index)
		{
			if (Worlds[Index] == World)
			{
				Worlds.RemoveAt(Index);
				break;
			}
		}

		World->DestroyWorld();
	}

	UFUNCTION(BlueprintCallable, Category = "Flecs")
	FORCEINLINE UFlecsWorld* GetFlecsWorld(const FString& Name) const
	{
		solid_checkf(HasWorld(Name), TEXT("World with name %s does not exist"), *Name);
		return WorldNameMap.at(Name);
	}
	
	UFUNCTION(BlueprintCallable, Category = "Flecs", Meta = (WorldContext = "WorldContextObject"))
	static FORCEINLINE UFlecsWorld* GetWorldStatic(UObject* WorldContextObject, const FString& Name)
	{
		return GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert)
		              ->GetSubsystem<UFlecsWorldSubsystem>()->GetFlecsWorld(Name);
	}

	UFUNCTION(BlueprintCallable, Category = "Flecs", Meta = (WorldContext = "WorldContextObject"))
	static FORCEINLINE UFlecsWorld* GetDefaultWorld(const UObject* WorldContextObject)
	{
		return GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert)
		              ->GetSubsystem<UFlecsWorldSubsystem>()->GetFlecsWorld(DEFAULT_FLECS_WORLD_NAME.data());
	}
	
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override
	{
		return WorldType == EWorldType::Game || WorldType == EWorldType::PIE || WorldType == EWorldType::GamePreview
			|| WorldType == EWorldType::GameRPC;
	}
	
	FOnWorldCreated OnWorldCreated;

protected:
	UPROPERTY()
	TArray<TObjectPtr<UFlecsWorld>> Worlds;

	robin_hood::unordered_flat_map<FString, UFlecsWorld*> WorldNameMap;

	UPROPERTY()
	TWeakObjectPtr<const UFlecsDeveloperSettings> DeveloperSettings;

	void RegisterAllGameplayTags(UFlecsWorld* InFlecsWorld)
	{
		TMap<FGameplayTag, TArray<FGameplayTag>> TagHierarchy;
		BuildTagHierarchyMap(TagHierarchy);

		TSet<FName> ProcessedTags;
		
		for (const TTuple<FGameplayTag, TArray<FGameplayTag>>& Pair : TagHierarchy)
		{
			if (!Pair.Key.IsValid())
			{
				continue;
			}
			
			RegisterGameplayTagEntityRecursively(Pair.Key, InFlecsWorld, TagHierarchy, ProcessedTags);
		}
	}

	void BuildTagHierarchyMap(TMap<FGameplayTag, TArray<FGameplayTag>>& InTagHierarchy)
	{
		FGameplayTagContainer AllTags;
		UGameplayTagsManager::Get().RequestAllGameplayTags(AllTags, false);

		for (const FGameplayTag& Tag : AllTags)
		{
			if (FGameplayTag ParentTag = Tag.RequestDirectParent(); ParentTag.IsValid())
			{
				InTagHierarchy.FindOrAdd(ParentTag).Add(Tag);
			}
			else
			{
				InTagHierarchy.FindOrAdd(Tag);
			}
		}
	}

	FFlecsEntityHandle RegisterGameplayTagEntityRecursively(const FGameplayTag& Tag, UFlecsWorld* NewFlecsWorld,
		const TMap<FGameplayTag, TArray<FGameplayTag>>& TagHierarchy, TSet<FName>& ProcessedTags)
	{
		if (ProcessedTags.Contains(Tag.GetTagName()))
		{
			return FFlecsEntityHandle();
		}
		
		ProcessedTags.Add(Tag.GetTagName());

		const FString LastPartOfTagName = ExtractLastPartOfTagName(Tag.GetTagName().ToString());

		const FFlecsEntityHandle TagEntity = NewFlecsWorld->CreateEntity(LastPartOfTagName);
		
		TagEntity.Set<FFlecsGameplayTagEntityComponent>({Tag});
		
		if (const TArray<FGameplayTag>* Children = TagHierarchy.Find(Tag))
		{
			for (const FGameplayTag& ChildTag : *Children)
			{
				FFlecsEntityHandle ChildEntity = RegisterGameplayTagEntityRecursively(ChildTag, NewFlecsWorld,
					TagHierarchy, ProcessedTags);
				
				ChildEntity.SetParent(TagEntity);
			}
		}

		return TagEntity;
	}

	FString ExtractLastPartOfTagName(const FString& FullTagName)
	{
		FString LastPartOfTagName;
		
		if (FullTagName.Split(TEXT("."), nullptr, &LastPartOfTagName,
			ESearchCase::IgnoreCase, ESearchDir::FromEnd))
		{
			return LastPartOfTagName;
		}
		
		return FullTagName;
	}
	
}; // class UFlecsWorldSubsystem
