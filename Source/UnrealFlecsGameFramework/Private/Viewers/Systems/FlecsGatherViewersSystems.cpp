// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Viewers/Systems/FlecsGatherViewersSystems.h"

#include "CoreGlobals.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "WorldPartition/WorldPartition.h"
#include "WorldPartition/WorldPartitionStreamingSource.h"

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
		// Resolve viewer membership before the OnLoad transform/perspective sync systems.
		.Phase(EFlecsPhaseType::PreFrame)
		.With<FFlecsViewerTrackerSingleton&>() // 0
		.With<const FFlecsViewerSubsystemSingleton>() // 1
		.Immediate();
}

void UFlecsGatherViewersSystem::RunEachIterator(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld,
	flecs::iter& InIterator)
{
	if UNLIKELY_IF(LastGatheredFrame == GFrameCounter)
	{
		return;
	}

	LastGatheredFrame = GFrameCounter;

	const TSolidNotNull<UWorld*> World = InWorld->GetWorld();
	const UWorldPartition* WorldPartition = World->GetWorldPartition();
	const TArray<FWorldPartitionStreamingSource>* StreamingSources = WorldPartition != nullptr
		? &WorldPartition->GetStreamingSources()
		: nullptr;
	
	FFlecsViewerTrackerSingleton& ViewerTrackerSingleton = InIterator.field_at<FFlecsViewerTrackerSingleton>(0, 0);
	
	const TSolidNotNull<UFlecsViewerWorldSubsystem*> ViewerSubsystem = InIterator.field_at<const FFlecsViewerSubsystemSingleton>(1, 0)
			.GetSubsystemChecked<UFlecsViewerWorldSubsystem>();
	
	const TSolidNotNull<const UFlecsGameFrameworkModuleSettings*> GameFrameworkModuleSettings = GetDefault<UFlecsGameFrameworkModuleSettings>();
	const bool bGatherPlayerControllers = GameFrameworkModuleSettings->bGatherPlayerControllers;
	const bool bGatherStreamingSources = GameFrameworkModuleSettings->bGatherStreamingSources;
	const bool bAllowNonPlayerViewerActors = GameFrameworkModuleSettings->bAllowNonPlayerViewerActors;

	TSet<TWeakObjectPtr<const APlayerController>> CurrentPlayerControllers;
	if (bGatherPlayerControllers)
	{
		CurrentPlayerControllers.Reserve(World->GetNumPlayerControllers());

		for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			const APlayerController* PlayerController = Iterator->Get();
			if LIKELY_IF(::IsValid(PlayerController))
			{
				CurrentPlayerControllers.Add(PlayerController);
			}
		}
	}

	TSet<FName> CurrentStreamingSources;
	if (bGatherStreamingSources && StreamingSources != nullptr)
	{
		CurrentStreamingSources.Reserve(StreamingSources->Num());
		for (const FWorldPartitionStreamingSource& StreamingSource : *StreamingSources)
		{
			if LIKELY_IF(!StreamingSource.Name.IsNone())
			{
				CurrentStreamingSources.Add(StreamingSource.Name);
			}
		}
	}

	TSet<FFlecsId> DestroyedViewerIds;

	const auto DestroyViewer = [&InIterator, &DestroyedViewerIds](const FFlecsEntityView& InViewer)
	{
		if LIKELY_IF(InViewer.IsValid())
		{
			const FFlecsId ViewerId(InViewer.GetRawId());
			if UNLIKELY_IF(DestroyedViewerIds.Contains(ViewerId))
			{
				return;
			}

			DestroyedViewerIds.Add(ViewerId);
			InViewer.ToMut<FFlecsEntityHandle>(InIterator).Destroy();
		}
	};

	// Keep the Flecs handles synchronized with the current engine-side sources. This
	// mirrors Mass LOD's stale-viewer pass while retaining the Flecs entity registry.
	for (auto It = ViewerTrackerSingleton.PCViewers.CreateIterator(); It; ++It)
	{
		if UNLIKELY_IF(!bGatherPlayerControllers
			|| !It.Key().IsValid()
			|| !CurrentPlayerControllers.Contains(It.Key())
			|| !It.Value().IsValid())
		{
			DestroyViewer(It.Value());
			It.RemoveCurrent();
		}
	}

	for (auto It = ViewerTrackerSingleton.StreamSourceViewers.CreateIterator(); It; ++It)
	{
		if UNLIKELY_IF(!bGatherStreamingSources
			|| !CurrentStreamingSources.Contains(It.Key())
			|| !It.Value().IsValid())
		{
			DestroyViewer(It.Value());
			It.RemoveCurrent();
		}
	}

	for (auto It = ViewerTrackerSingleton.ActorViewers.CreateIterator(); It; ++It)
	{
		if UNLIKELY_IF(!bAllowNonPlayerViewerActors
			|| !It.Key().IsValid()
			|| !It.Value().IsValid())
		{
			DestroyViewer(It.Value());
			It.RemoveCurrent();
		}
	}

	if LIKELY_IF(ViewerQuery.IsValid())
	{
		ViewerQuery.iter(InIterator).each([&](flecs::entity Entity)
		{
			const FFlecsEntityView Viewer(Entity);

			if (const FFlecsViewerPlayerComponent* PlayerComponent = Viewer.TryGet<FFlecsViewerPlayerComponent>())
			{
				const APlayerController* PlayerController = PlayerComponent->PlayerController.Get();
				const TWeakObjectPtr<const APlayerController> PlayerControllerKey = PlayerController;

				if UNLIKELY_IF(!bGatherPlayerControllers || !::IsValid(PlayerController)
					|| !CurrentPlayerControllers.Contains(PlayerControllerKey))
				{
					DestroyViewer(Viewer);
					return;
				}

				if (const FFlecsEntityView* ExistingViewer = ViewerTrackerSingleton.PCViewers.Find(PlayerControllerKey))
				{
					if (ExistingViewer->IsValid() && ExistingViewer->GetRawId() != Viewer.GetRawId())
					{
						DestroyViewer(Viewer);
						return;
					}
				}

				ViewerTrackerSingleton.PCViewers.Add(PlayerControllerKey, Viewer);
				return;
			}

			if (const FFlecsViewerStreamingSourceComponent* StreamingSourceComponent
				= Viewer.TryGet<FFlecsViewerStreamingSourceComponent>())
			{
				const FName StreamingSourceName = StreamingSourceComponent->StreamingSourceName;
				if UNLIKELY_IF(!bGatherStreamingSources
					|| StreamingSourceName.IsNone()
					|| !CurrentStreamingSources.Contains(StreamingSourceName))
				{
					DestroyViewer(Viewer);
					return;
				}

				if (const FFlecsEntityView* ExistingViewer = ViewerTrackerSingleton.StreamSourceViewers.Find(StreamingSourceName))
				{
					if (ExistingViewer->IsValid() && ExistingViewer->GetRawId() != Viewer.GetRawId())
					{
						DestroyViewer(Viewer);
						return;
					}
				}

				ViewerTrackerSingleton.StreamSourceViewers.Add(StreamingSourceName, Viewer);
				return;
			}

			if (const FFlecsViewerActorComponent* ActorComponent = Viewer.TryGet<FFlecsViewerActorComponent>())
			{
				const AActor* Actor = ActorComponent->Actor.Get();
				const TWeakObjectPtr<const AActor> ActorKey = Actor;

				if UNLIKELY_IF(!bAllowNonPlayerViewerActors || !::IsValid(Actor))
				{
					DestroyViewer(Viewer);
					return;
				}

				if (FFlecsEntityView* ExistingViewer = ViewerTrackerSingleton.ActorViewers.Find(ActorKey))
				{
					if (ExistingViewer->IsValid() && ExistingViewer->GetRawId() != Viewer.GetRawId())
					{
						DestroyViewer(Viewer);
						return;
					}
				}

				ViewerTrackerSingleton.ActorViewers.Add(ActorKey, Viewer);
				return;
			}

			DestroyViewer(Viewer);
		});
	}

	if (bGatherPlayerControllers)
	{
		for (const TWeakObjectPtr<const APlayerController>& PlayerControllerKey : CurrentPlayerControllers)
		{
			FFlecsEntityView* ExistingViewer = ViewerTrackerSingleton.PCViewers.Find(PlayerControllerKey);
			if (ExistingViewer != nullptr && ExistingViewer->IsValid())
			{
				continue;
			}

			const APlayerController* PlayerController = PlayerControllerKey.Get();
			if UNLIKELY_IF(!::IsValid(PlayerController))
			{
				continue;
			}

			const TSolidNotNull<const APlayerController*> NonNullPlayerController = PlayerController;
			ViewerTrackerSingleton.PCViewers.Add(PlayerControllerKey,
				ViewerSubsystem->CreatePlayerViewer(NonNullPlayerController));
		}
	}

	if (bGatherStreamingSources)
	{
		for (const FName& StreamingSourceName : CurrentStreamingSources)
		{
			FFlecsEntityView* ExistingViewer = ViewerTrackerSingleton.StreamSourceViewers.Find(StreamingSourceName);
			if (ExistingViewer != nullptr && ExistingViewer->IsValid())
			{
				continue;
			}

			ViewerTrackerSingleton.StreamSourceViewers.Add(StreamingSourceName,
				ViewerSubsystem->AddStreamSourceViewer(StreamingSourceName));
		}
	}
}

void UFlecsGatherViewersSystem::RegisterObject(const TSolidNotNull<UFlecsWorldInterfaceObject*> InFlecsWorld)
{
	Super::RegisterObject(InFlecsWorld);

	InFlecsWorld->RegisterComponentType<FFlecsViewerTrackerSingleton>();
	InFlecsWorld->Set<FFlecsViewerTrackerSingleton>(FFlecsViewerTrackerSingleton{});
}

void UFlecsGatherViewersSystem::FlecsWorldBeginPlay(const TSolidNotNull<UFlecsWorldInterfaceObject*> InFlecsWorld)
{
	Super::FlecsWorldBeginPlay(InFlecsWorld);
	LastGatheredFrame = TNumericLimits<uint64>::Max();

	ViewerQuery = InFlecsWorld->CreateQueryBuilder("GatherViewersSystem_ViewerQuery")
		.WithPair<FFlecsViewerRelationship>(flecs::Wildcard) // 0
		.Without(flecs::Prefab)
		.Build();
	
	ViewerQuery.GetEntity().SetChildOf(GetSystemHandle());
}

void UFlecsGatherViewersSystem::UnregisterObject(const TSolidNotNull<UFlecsWorldInterfaceObject*> InFlecsWorld)
{
	ViewerQuery.Destroy();
	
	Super::UnregisterObject(InFlecsWorld);
}
