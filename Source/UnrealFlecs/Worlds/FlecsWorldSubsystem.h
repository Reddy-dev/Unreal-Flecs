﻿// Solstice Games © 2024. All Rights Reserved.

// ReSharper disable CppMemberFunctionMayBeStatic
// ReSharper disable CppExpressionWithoutSideEffects
// ReSharper disable CppUnusedIncludeDirective
#pragma once

#include <thread>
#include <unordered_map>

#include "CoreMinimal.h"
#include "flecs.h"
#include "FlecsWorld.h"
#include "FlecsWorldInfoSettings.h"
#include "FlecsWorldSettingsAsset.h"
#include "FlecsWorldSettings.h"
#include "GameplayTagContainer.h"
#include "GameplayTagsManager.h"
#include "Components/FlecsWorldPtrComponent.h"
#include "Components/UWorldPtrComponent.h"
#include "Entities/FlecsDefaultEntityEngineSubsystem.h"
#include "GameFramework/GameStateBase.h"
#include "General/FlecsDeveloperSettings.h"
#include "SolidMacros/Concepts/SolidConcepts.h"
#include "SolidMacros/Standard/Hashing.h"
#include "Standard/robin_hood.h"
#include "Subsystems/WorldSubsystem.h"
#include "FlecsWorldSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnWorldCreated, UFlecsWorld*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnWorldBeginPlay, UWorld*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnWorldDestroyed, UFlecsWorld*);

UCLASS(BlueprintType)
class UNREALFLECS_API UFlecsWorldSubsystem final : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override
	{
		return Super::ShouldCreateSubsystem(Outer) && GetDefault<UFlecsDeveloperSettings>()->bEnableFlecs;
	}
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override
	{
		Super::Initialize(Collection);
		
		DeveloperSettings = GetDefault<UFlecsDeveloperSettings>();
		
		if (!DeveloperSettings->bEnableFlecs)
		{
			return;
		}

		SetTickableTickType(ETickableTickType::Always);

		solid_check(IsValid(GetWorld()->GetWorldSettings()));
		checkf(GetWorld()->GetWorldSettings()->IsA<AFlecsWorldSettings>(),
			TEXT("World settings must be of type AFlecsWorldSettings"));

		const AFlecsWorldSettings* SettingsActor = CastChecked<AFlecsWorldSettings>(GetWorld()->GetWorldSettings());

		if (const UFlecsWorldSettingsAsset* SettingsAsset = SettingsActor->DefaultWorld)
		{
			CreateWorld(SettingsAsset->WorldSettings.WorldName, SettingsAsset->WorldSettings);
		}
	}

	virtual void OnWorldBeginPlay(UWorld& InWorld) override
	{
		Super::OnWorldBeginPlay(InWorld);

		if LIKELY_IF(IsValid(DefaultWorld))
		{
			DefaultWorld->WorldBeginPlay();
			OnWorldBeginPlayDelegate.Broadcast(&InWorld);
		}
	}

	virtual void Deinitialize() override
	{
		if (IsValid(DefaultWorld))
		{
			DefaultWorld->RemoveSingleton<FFlecsWorldPtrComponent>();
			DefaultWorld->RemoveSingleton<FUWorldPtrComponent>();
			DefaultWorld->DestroyWorld();
		}

		DefaultWorld = nullptr;

		Super::Deinitialize();
	}

	FORCEINLINE virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(UFlecsWorldSubsystem, STATGROUP_Tickables);
	}

	virtual void Tick(float DeltaTime) override
	{
		Super::Tick(DeltaTime);

		if UNLIKELY_IF(!IsValid(DefaultWorld))
		{
			return;
		}
		
		const bool bResult = DefaultWorld->Progress(DeltaTime);

		#if WITH_EDITOR

		if UNLIKELY_IF(!bResult)
		{
			UN_LOGF(LogFlecsCore, Error, "Failed to progress Flecs world");
		}

		#endif // WITH_EDITOR
	}
	
	UFUNCTION()
	FORCEINLINE UFlecsWorld* CreateWorld(const FString& Name, const FFlecsWorldSettingsInfo& Settings)
	{
		solid_checkf(!Name.IsEmpty(), TEXT("World name cannot be NAME_None"));

		SetTickableTickType(ETickableTickType::Always);
		
		TArray<FFlecsDefaultMetaEntity> DefaultEntities = FFlecsDefaultEntityEngine::Get().AddedDefaultEntities;
		TMap<FString, flecs::entity_t> DefaultEntityIds = FFlecsDefaultEntityEngine::Get().DefaultEntityOptions;
		
		// Add a the debug string for this world to the passed-in name E.G. "MyWorld (Client)"
		const FName WorldNameWithWorldContext = FName(Name +" ("+ GetDebugStringForWorld(GetWorld())+")");
		
		UFlecsWorld* NewFlecsWorld = NewObject<UFlecsWorld>(this, WorldNameWithWorldContext);
		
		DefaultWorld = NewFlecsWorld;
		
		NewFlecsWorld->SetContext(this);

		NewFlecsWorld->SetSingleton<FFlecsWorldPtrComponent>(FFlecsWorldPtrComponent{ NewFlecsWorld });
		NewFlecsWorld->SetSingleton<FUWorldPtrComponent>(FUWorldPtrComponent{ GetWorld() });
		
		DefaultWorld->InitializeDefaultComponents();
		
		for (int32 Index = 0; Index < DefaultEntities.Num(); ++Index)
		{
			FString EntityName = DefaultEntities[Index].EntityName;
			const flecs::entity_t EntityId = DefaultEntityIds[EntityName];

			#if WITH_EDITOR
			
			FFlecsEntityHandle NewEntity =
				
			#endif // WITH_EDITOR
				
			NewFlecsWorld->CreateEntityWithRecordWithId(DefaultEntities[Index].EntityRecord, EntityId);

			UN_LOGF(LogFlecsCore, Log,
				"Created default entity %s with id %d", *EntityName, EntityId);
			
			UN_LOGF(LogFlecsCore, Log,
				"Entity %s with id %d", *NewEntity.GetName(), NewEntity.GetId());
		}

		if (DeveloperSettings->bUseTaskThreads)
		{
			NewFlecsWorld->SetTaskThreads(DeveloperSettings->TaskThreadCount);
		}
		else
		{
			NewFlecsWorld->SetThreads(std::thread::hardware_concurrency());
		}

		NewFlecsWorld->WorldStart();

		RegisterAllGameplayTags(NewFlecsWorld);

		for (UObject* Module : Settings.Modules)
		{
			solid_checkf(Module->GetClass()->ImplementsInterface(UFlecsModuleInterface::StaticClass()),
				TEXT("Module %s does not implement UFlecsModuleInterface"), *Module->GetName());
			
			NewFlecsWorld->ImportModule(Module);
		}
		
		OnWorldCreatedDelegate.Broadcast(NewFlecsWorld);
		
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
			? IsValid(GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert)
				->GetSubsystem<UFlecsWorldSubsystem>()->DefaultWorld)
			: false;
	}
	
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override
	{
		return WorldType == EWorldType::Game
			|| WorldType == EWorldType::PIE
			|| WorldType == EWorldType::GameRPC;
	}
	
	FOnWorldCreated OnWorldCreatedDelegate;
	FOnWorldBeginPlay OnWorldBeginPlayDelegate;
	FOnWorldDestroyed OnWorldDestroyedDelegate;

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
			return FFlecsEntityHandle::GetNullHandle();
		}
		
		ProcessedTags.Add(Tag.GetTagName());

		const FString LastPartOfTagName = ExtractLastPartOfTagName(Tag.GetTagName().ToString());

		const FFlecsEntityHandle TagEntity = NewFlecsWorld->CreateEntity(LastPartOfTagName);
		
		TagEntity.Set<FGameplayTag>(Tag);
		
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
		if (FullTagName.IsEmpty())
		{
			return FString();
		}

		if (int32 LastDotIndex; FullTagName.FindLastChar(TEXT('.'), LastDotIndex))
		{
			return FullTagName.RightChop(LastDotIndex + 1);
		}
		
		return FullTagName;
	}
	
}; // class UFlecsWorldSubsystem
