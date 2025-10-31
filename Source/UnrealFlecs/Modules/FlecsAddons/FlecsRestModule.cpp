﻿// Elie Wiese-Namir © 2025. All Rights Reserved.

// ReSharper disable CppExpressionWithoutSideEffects
#include "FlecsRestModule.h"

#include "Engine/EngineBaseTypes.h"
#include "Engine/World.h"

#include "Worlds/FlecsWorld.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsRestModule)

void UFlecsRestModule::InitializeModule(TSolidNotNull<UFlecsWorld*> InWorld, const FFlecsEntityHandle& InModuleEntity)
{
	// End the scope as we don't want to import a module inside another module's scope
	InWorld->EndScope([this, InWorld]()
	{
#ifdef FLECS_REST
	
		uint16 ClientPieInstanceOffset = 0;
		
		const TSolidNotNull<const UWorld*> UnrealWorld = InWorld->GetWorld();

		#if WITH_EDITOR
		
		if (UnrealWorld->GetNetMode() == NM_Client)
		{
			ClientPieInstanceOffset = static_cast<uint16>(UE::GetPlayInEditorID());
		}
		
		#endif // WITH_EDITOR
		
		const uint16 RestPort = ECS_REST_DEFAULT_PORT + ClientPieInstanceOffset;

		InWorld->SetSingleton<flecs::Rest>(flecs::Rest{ .port = RestPort });
		
		RestEntity = InWorld->ObtainSingletonEntity<flecs::Rest>();
		ensureAlwaysMsgf(RestEntity.IsValid(), TEXT("Failed to create Flecs Rest entity with port %d"), RestPort);

		#ifdef FLECS_STATS

		if (bImportStats)
		{
			StatsEntity = InWorld->ImportFlecsModule<flecs::stats>();
		}

		#endif // #ifdef FLECS_STATS

#endif // #ifdef FLECS_REST
	});
}

void UFlecsRestModule::DeinitializeModule(TSolidNotNull<UFlecsWorld*> InWorld)
{
	// @TODO: Add Tests for destroying this module during runtime
#ifdef FLECS_REST

#ifdef FLECS_STATS
	
	if (StatsEntity.IsValid())
	{
		StatsEntity.Destroy();
	}

#endif // FLECS_STATS
	
	if UNLIKELY_IF(!RestEntity.IsValid())
	{
		return;
	}
	
	RestEntity.Destroy();

#endif // FLECS_REST
}
