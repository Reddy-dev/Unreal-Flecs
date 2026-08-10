// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Viewers/Systems/FlecsGatherViewersSystems.h"

#include "FlecsGameFrameworkModuleSettings.h"
#include "Viewers/Components/FlecsViewerTrackerSingleton.h"
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
		.With<FFlecsViewerTrackerSingleton&>(); // 0
}

void UFlecsGatherViewersSystem::RunEachIterator(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld,
	flecs::iter& InIterator)
{
	const TSolidNotNull<UWorld*> World = InWorld->GetWorld();
	const TSolidNotNull<UWorldPartition*> WorldPartition = World->GetWorldPartition();
	
	FFlecsViewerTrackerSingleton& ViewerTrackerSingleton = InIterator.field_at<FFlecsViewerTrackerSingleton>(0, 0);
	
	const TSolidNotNull<const UFlecsGameFrameworkModuleSettings*> GameFrameworkModuleSettings = GetDefault<UFlecsGameFrameworkModuleSettings>();
	
	if (GameFrameworkModuleSettings->bGatherPlayerControllers)
	{
		for (const TTuple<TWeakObjectPtr<const APlayerController>, FFlecsEntityView> Pair : ViewerTrackerSingleton.PCViewers)
		{
			
				
		}
			
		for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			const TSolidNotNull<const APlayerController*> PlayerController = Iterator->Get();
			
			
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
