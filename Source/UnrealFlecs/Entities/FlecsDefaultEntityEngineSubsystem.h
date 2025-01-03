﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FlecsDefaultEntitiesDeveloperSettings.h"
#include "flecs.h"

struct UNREALFLECS_API FFlecsDefaultEntityEngine final
{
	static FFlecsDefaultEntityEngine& Get()
	{
		static FFlecsDefaultEntityEngine Instance;
		return Instance;
	}

	friend class FUnrealFlecsModule;
	
public:
	FFlecsDefaultEntityEngine();
	
	void RefreshDefaultEntities();
	
	flecs::entity_t AddDefaultEntity(const FFlecsDefaultMetaEntity& DefaultEntity);
	
	TMap<FString, flecs::entity_t> DefaultEntityOptions;
	TArray<FFlecsDefaultMetaEntity> AddedDefaultEntities;
	TArray<FFlecsDefaultMetaEntity> CodeAddedDefaultEntities;
	
	flecs::world* DefaultEntityWorld = nullptr;
	flecs::entity NoneEntity;

	bool bIsInitialized = false;
	
	void Initialize();
}; // struct FFlecsDefaultEntityEngine

#define DEFINE_DEFAULT_ENTITY_OPTION(DefaultEntityName) \
	DLLEXPORT INLINE ECS_ENTITY_DECLARE(DefaultEntityName); \
	namespace \
	{                                                             \
		static void Register##DefaultEntityName()                        \
		{                                                         \
			FFlecsDefaultMetaEntity MetaEntity;                   \
			MetaEntity.EntityName = TEXT(#DefaultEntityName);      \
			if (!FFlecsDefaultEntityEngine::Get().bIsInitialized)  \
			{                                                     \
				FFlecsDefaultEntityEngine::Get().Initialize();                                   \
			}                                                     \
			DefaultEntityName = FFlecsDefaultEntityEngine::Get().AddDefaultEntity(MetaEntity); \
		}                                                         \
		struct FRegisterInvoker##DefaultEntityName                       \
		{                                                         \
			FRegisterInvoker##DefaultEntityName()                        \
			{                                                     \
				FCoreDelegates::OnPostEngineInit.AddStatic(&Register##DefaultEntityName); \
			}                                                     \
		};                                                        \
		inline FRegisterInvoker##DefaultEntityName Invoker##DefaultEntityName;  \
	}

