﻿// Solstice Games © 2024. All Rights Reserved.

// ReSharper disable CppMemberFunctionMayBeStatic
// ReSharper disable CppExpressionWithoutSideEffects
// ReSharper disable CppUnusedIncludeDirective
#pragma once

#include <unordered_map>

#include "CoreMinimal.h"
#include "flecs.h"
#include "FlecsWorld.h"
#include "FlecsWorldSettings.h"
#include "FlecsWorldSettingsAsset.h"
#include "FlecsWorldStartComponent.h"
#include "GameplayTagContainer.h"
#include "GameplayTagsManager.h"
#include "Components/FlecsGameplayTagEntityComponent.h"
#include "Components/FlecsWorldPtrComponent.h"
#include "Components/UWorldPtrComponent.h"
#include "Entities/FlecsDefaultEntityEngineSubsystem.h"
#include "GameFramework/GameStateBase.h"
#include "General/FlecsDeveloperSettings.h"
#include "SolidMacros/Concepts/SolidConcepts.h"
#include "SolidMacros/Standard/Hashing.h"
#include "Standard/robin_hood.h"
#include "Subsystems/WorldSubsystem.h"
#include "Unlog/Target/MessageLog.h"
#include "FlecsWorldSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnWorldCreated, const FString&, UFlecsWorld*);

UCLASS(BlueprintType)
class UNREALFLECS_API UFlecsWorldSubsystem final : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override
	{
		return GetDefault<UFlecsDeveloperSettings>()->bEnableFlecs;
	}

	virtual ETickableTickType GetTickableTickType() const override
	{
		return ETickableTickType::Always;
	}
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override
	{
		Super::Initialize(Collection);
		
		DeveloperSettings = GetDefault<UFlecsDeveloperSettings>();
	}

	virtual void OnWorldBeginPlay(UWorld& InWorld) override
	{
		Super::OnWorldBeginPlay(InWorld);

		const AGameStateBase* GameState = InWorld.GetGameState();
		
		if UNLIKELY_IF(!IsValid(GameState))
		{
			return;
		}

		const UFlecsWorldStartComponent* StartComponent = GameState->FindComponentByClass<UFlecsWorldStartComponent>();
		solid_checkf(IsValid(StartComponent), TEXT("StartComponent must be valid"));

		CreateWorld(StartComponent->DefaultWorld->WorldSettings.WorldName, StartComponent->DefaultWorld->WorldSettings);
	}

	virtual void Deinitialize() override
	{
		Super::Deinitialize();

		if (IsValid(DefaultWorld))
		{
			DefaultWorld->DestroyWorld();
		}
	}

	FORCEINLINE virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(UFlecsWorldSubsystem, STATGROUP_Tickables);
	}

	virtual void Tick(float DeltaTime) override
	{
		const bool bResult = DefaultWorld->Progress(DeltaTime);

		if UNLIKELY_IF(!bResult)
		{
			UN_LOGF(LogFlecsCore, Error, "Failed to progress Flecs world");
		}
	}
	
	UFUNCTION()
	FORCEINLINE UFlecsWorld* CreateWorld(const FString& Name, const FFlecsWorldSettings& Settings)
	{
		solid_checkf(!Name.IsEmpty(), TEXT("World name cannot be NAME_None"));

		flecs::world NewWorld = flecs::world();
		
		TArray<FFlecsDefaultMetaEntity> DefaultEntities = FFlecsDefaultEntityEngine::Get().AddedDefaultEntities;
		TMap<FString, flecs::entity_t> DefaultEntityIds = FFlecsDefaultEntityEngine::Get().DefaultEntityOptions;

		UFlecsWorld* NewFlecsWorld = NewObject<UFlecsWorld>(this);
		NewFlecsWorld->SetWorld(std::move(NewWorld));
		
		DefaultWorld = NewFlecsWorld;
		
		GetDefaultWorld()->AddSingleton<FFlecsTypeMapComponent>();
		GetDefaultWorld()->TypeMapComponent = GetDefaultWorld()->GetSingletonFlecsRef<FFlecsTypeMapComponent>();

		NewFlecsWorld->SetSingleton<FFlecsWorldPtrComponent>(
			FFlecsWorldPtrComponent { NewFlecsWorld });

		NewFlecsWorld->SetSingleton<FUWorldPtrComponent>(FUWorldPtrComponent { GetWorld() });

		for (int32 Index = 0; Index < DefaultEntities.Num(); ++Index)
		{
			FString EntityName = DefaultEntities[Index].EntityRecord.Name;
			const flecs::entity_t EntityId = DefaultEntityIds[EntityName];

			#if WITH_EDITOR
			FFlecsEntityHandle NewEntity =
			#endif // WITH_EDITOR
			NewFlecsWorld->CreateEntityWithRecordWithId(DefaultEntities[Index].EntityRecord, EntityId);

			#if WITH_EDITOR

			UN_LOGF(LogFlecsCore, Log,
				"Created default entity %s with id %d", *EntityName, EntityId);
			
			UN_LOGF(LogFlecsCore, Log,
				"Entity %s with id %d", *NewEntity.GetName(), NewEntity.GetId());

			#endif // WITH_EDITOR
		}

		NewFlecsWorld->SetWorldName(Name);
		
		NewFlecsWorld->SetContext(this);

		NewFlecsWorld->SetThreads(Settings.DefaultWorkerThreads);

		NewFlecsWorld->WorldBeginPlay();

		RegisterAllGameplayTags(NewFlecsWorld);

		for (UObject* Module : Settings.Modules)
		{
			solid_checkf(Module->GetClass()->ImplementsInterface(UFlecsModuleInterface::StaticClass()),
				TEXT("Module %s does not implement UFlecsModuleInterface"), *Module->GetName());
			
			NewFlecsWorld->ImportModule(Module);
		}

		NewFlecsWorld->Progress();
		
		OnWorldCreated.Broadcast(Name, NewFlecsWorld);
		
		return NewFlecsWorld;
	}

	UFUNCTION(BlueprintCallable, Category = "Flecs")
	FORCEINLINE UFlecsWorld* GetDefaultWorld() const
	{
		return DefaultWorld;
	}

	UFUNCTION(BlueprintCallable, Category = "Flecs")
	FORCEINLINE bool HasValidFlecsWorld() const
	{
		return IsValid(DefaultWorld);
	}

	UFUNCTION(BlueprintCallable, Category = "Flecs", Meta = (WorldContext = "WorldContextObject"))
	static FORCEINLINE UFlecsWorld* GetDefaultWorldStatic(const UObject* WorldContextObject)
	{
		return GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert)
			? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert)
				->GetSubsystem<UFlecsWorldSubsystem>()->DefaultWorld
			: nullptr;
	}

	UFUNCTION(BlueprintCallable, Category = "Flecs", Meta = (WorldContext = "WorldContextObject"))
	static FORCEINLINE bool HasValidFlecsWorldStatic(const UObject* WorldContextObject)
	{
		return GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull)
			? IsValid(GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull)
				->GetSubsystem<UFlecsWorldSubsystem>()->DefaultWorld)
			: false;
	}
	
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override
	{
		return WorldType == EWorldType::Game || WorldType == EWorldType::PIE || WorldType == EWorldType::GamePreview
			|| WorldType == EWorldType::GameRPC;
	}
	
	FOnWorldCreated OnWorldCreated;

protected:
	UPROPERTY()
	TObjectPtr<UFlecsWorld> DefaultWorld;

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
