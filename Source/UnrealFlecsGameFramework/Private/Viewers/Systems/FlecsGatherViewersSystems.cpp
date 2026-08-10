// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Viewers/Systems/FlecsGatherViewersSystems.h"

#include "FlecsGameFrameworkModuleSettings.h"
#include "Viewers/FlecsViewerWorldSubsystem.h"
#include "Viewers/Components/FlecsViewerSubsystemSingleton.h"
#include "Viewers/Components/FlecsViewerTrackerSingleton.h"
#include "Viewers/Components/FlecsViewerTypeComponents.h"
#include "Worlds/FlecsWorldInterfaceObject.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsGatherViewersSystems)

UFlecsGatherViewersSystem::UFlecsGatherViewersSystem()
{
}

void UFlecsGatherViewersSystem::BuildSystem(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld,
                                            TFlecsSystemBuilder<>& InBuilder) const
{
	InBuilder
		.Phase(EFlecsPhaseType::PreUpdate)
		.With<FFlecsViewerTrackerSingleton&>() // 0
		.With<const FFlecsViewerSubsystemSingleton>(); // 1
}

void UFlecsGatherViewersSystem::RunEachIterator(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld,
	flecs::iter& InIterator)
{
	const TSolidNotNull<UWorld*> World = InWorld->GetWorld();
	const TSolidNotNull<UWorldPartition*> WorldPartition = World->GetWorldPartition();
	
	FFlecsViewerTrackerSingleton& ViewerTrackerSingleton = InIterator.field_at<FFlecsViewerTrackerSingleton>(0, 0);
	
	const TSolidNotNull<UFlecsViewerWorldSubsystem*> ViewerSubsystem = InIterator.field_at<const FFlecsViewerSubsystemSingleton>(1, 0)
			.GetSubsystemChecked<UFlecsViewerWorldSubsystem>();
	
	const TSolidNotNull<const UFlecsGameFrameworkModuleSettings*> GameFrameworkModuleSettings = GetDefault<UFlecsGameFrameworkModuleSettings>();
	
	if (GameFrameworkModuleSettings->bGatherPlayerControllers)
	{
		for (int32 Index = ViewerTrackerSingleton.PCViewers.Num() - 1; Index < ViewerTrackerSingleton.PCViewers.Num(); --Index)
		{
			solid_cassume(Index >= 0);
			
			TPair<TWeakObjectPtr<const APlayerController>, FFlecsEntityView>& Pair 
				= ViewerTrackerSingleton.PCViewers.Get(FSetElementId::FromInteger(Index));
			
			if (!Pair.Key.IsValid())
			{
				Pair.Value.ToMut<FFlecsEntityHandle>(InIterator).Destroy();
			}
		}
			
		for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			const TSolidNotNull<const APlayerController*> PlayerController = Iterator->Get();
			
			if (ViewerTrackerSingleton.PCViewers.Contains(PlayerController))
			{
				continue;
			}
			
			ViewerTrackerSingleton.PCViewers.Emplace(PlayerController, 
				ViewerSubsystem->CreatePlayerViewer(PlayerController));
		}
	}
	
	if (GameFrameworkModuleSettings->bGatherStreamingSources)
	{
		for (
	}
	
	if (GameFrameworkModuleSettings->bAllowNonPlayerViewerActors)
	{
		for (
	}
	
}

void UFlecsGatherViewersSystem::RegisterObject(const TSolidNotNull<UFlecsWorldInterfaceObject*> InFlecsWorld)
{
	Super::RegisterObject(InFlecsWorld);
}

void UFlecsGatherViewersSystem::FlecsWorldBeginPlay(const TSolidNotNull<UFlecsWorldInterfaceObject*> InFlecsWorld)
{
	ViewerQuery = InFlecsWorld->CreateQueryBuilder("GatherViewersSystem_ViewerQuery")
		.WithPair<FFlecsViewerRelationship>(flecs::Wildcard) // 0
		.Build();
	
	ViewerQuery.GetEntity().SetChildOf(GetSystemHandle());
	
	Super::FlecsWorldBeginPlay(InFlecsWorld);
}

void UFlecsGatherViewersSystem::UnregisterObject(const TSolidNotNull<UFlecsWorldInterfaceObject*> InFlecsWorld)
{
	ViewerQuery.Destroy();
	
	Super::UnregisterObject(InFlecsWorld);
}
