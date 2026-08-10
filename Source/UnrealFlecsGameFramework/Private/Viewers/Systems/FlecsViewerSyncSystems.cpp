// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Viewers/Systems/FlecsViewerSyncSystems.h"

#include "WorldPartition/WorldPartition.h"

#include "FlecsGameFrameworkModuleSettings.h"
#include "Components/FlecsWorldPtrComponent.h"

#include "Viewers/Components/FlecsViewerPerspectiveComponent.h"
#include "Viewers/Components/FlecsViewerTransformComponent.h"
#include "Viewers/Components/FlecsViewerTypeComponents.h"
#include "Viewers/Systems/FlecsGatherViewersSystems.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsViewerSyncSystems)

UFlecsPlayerViewerSyncSystem::UFlecsPlayerViewerSyncSystem()
{
}

void UFlecsPlayerViewerSyncSystem::BuildSystem(const TSolidNotNull<const UFlecsWorldInterfaceObject*> FlecsWorldInterfaceObject,
	TFlecsSystemBuilder<>& InBuilder) const
{
	InBuilder
		.Phase(EFlecsPhaseType::PreFrame)
		.TickSource(UFlecsGatherViewersSystem::StaticClass())
		.WithPair<FFlecsViewerRelationship, const FFlecsViewerPlayerComponent>() // 0
		.With<FFlecsViewerTransformComponent&>() // 1
		.With<FFlecsViewerPerspectiveComponent&>(); // 2
}

void UFlecsPlayerViewerSyncSystem::EachIterator(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld,
	flecs::iter& InIterator, const FFlecsId InIndex)
{
	// @TODO: Ignore Player Controllers due to simulation
	const TSolidNotNull<const UFlecsGameFrameworkModuleSettings*> GameFrameworkSettings = GetDefault<UFlecsGameFrameworkModuleSettings>();
	const bool bUsePlayerPawnLocationInsteadOfCamera = GameFrameworkSettings->bUsePlayerPawnLocationInsteadOfCamera;
	
	const FFlecsViewerPlayerComponent& PlayerComponent = InIterator.field_at<const FFlecsViewerPlayerComponent>(0, InIndex);
	
	FFlecsViewerTransformComponent& TransformComponent = InIterator.field_at<FFlecsViewerTransformComponent>(1, InIndex);
	FFlecsViewerPerspectiveComponent& PerspectiveComponent = InIterator.field_at<FFlecsViewerPerspectiveComponent>(2, InIndex);
	
	const TSolidNotNull<APlayerController*> PlayerController = PlayerComponent.PlayerController;
	
	if (bUsePlayerPawnLocationInsteadOfCamera)
	{
		TransformComponent.Location = PlayerController->GetPawn()->GetActorLocation();
		TransformComponent.Rotation = PlayerController->GetPawn()->GetActorRotation();
	}
	else
	{
		FVector PlayerCameraLocation(ForceInitToZero);
		FRotator PlayerCameraRotation(FRotator::ZeroRotator);
		
		PlayerController->GetPlayerViewPoint(PlayerCameraLocation, PlayerCameraRotation);
		
		TransformComponent.Location = PlayerCameraLocation;
		TransformComponent.Rotation = PlayerCameraRotation;
	}
}

UFlecsActorViewerSyncSystem::UFlecsActorViewerSyncSystem()
{
}

void UFlecsActorViewerSyncSystem::BuildSystem(
	const TSolidNotNull<const UFlecsWorldInterfaceObject*> FlecsWorldInterfaceObject,
	TFlecsSystemBuilder<>& InBuilder) const
{
	InBuilder
		.Phase(EFlecsPhaseType::PreFrame)
		.TickSource(UFlecsGatherViewersSystem::StaticClass())
		.WithPair<FFlecsViewerRelationship, const FFlecsViewerActorComponent>() // 0
		.With<FFlecsViewerTransformComponent&>(); // 1
}

void UFlecsActorViewerSyncSystem::EachIterator(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld,
	flecs::iter& InIterator, const FFlecsId InIndex)
{
	const FFlecsViewerActorComponent& ActorComponent = InIterator.field_at<const FFlecsViewerActorComponent>(0, InIndex);
	FFlecsViewerTransformComponent& TransformComponent = InIterator.field_at<FFlecsViewerTransformComponent>(1, InIndex);
	
	TransformComponent.Location = ActorComponent.Actor->GetActorLocation();
	TransformComponent.Rotation = ActorComponent.Actor->GetActorRotation();
}

UFlecsStreamingSourceViewerSyncSystem::UFlecsStreamingSourceViewerSyncSystem()
{
}

void UFlecsStreamingSourceViewerSyncSystem::BuildSystem(const TSolidNotNull<const UFlecsWorldInterfaceObject*> FlecsWorldInterfaceObject,
	TFlecsSystemBuilder<>& InBuilder) const
{
	InBuilder
		.Phase(EFlecsPhaseType::PreFrame)
		.TickSource(UFlecsGatherViewersSystem::StaticClass())
		.WithPair<FFlecsViewerRelationship, const FFlecsViewerStreamingSourceComponent>() // 0
		.With<FFlecsViewerTransformComponent&>(); // 1
}

void UFlecsStreamingSourceViewerSyncSystem::EachIterator(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld,
	flecs::iter& InIterator, const FFlecsId InIndex)
{
	const TSolidNotNull<const UWorldPartition*> WorldPartition = InWorld->GetWorld()->GetWorldPartition();
	const TArray<FWorldPartitionStreamingSource>& StreamingSources = WorldPartition->GetStreamingSources();
	
	const FFlecsViewerStreamingSourceComponent& StreamingSourceComponent 
		= InIterator.field_at<const FFlecsViewerStreamingSourceComponent>(0, InIndex);
	FFlecsViewerTransformComponent& TransformComponent 
		= InIterator.field_at<FFlecsViewerTransformComponent>(1, InIndex);
	
	const FWorldPartitionStreamingSource* StreamingSource = StreamingSources.FindByPredicate(
	[&](const FWorldPartitionStreamingSource& Source)
	{
		return Source.Name == StreamingSourceComponent.StreamingSourceName;
	});
	
	if UNLIKELY_IF(!StreamingSource)
	{
		TransformComponent.Location = FVector::ZeroVector;
		TransformComponent.Rotation = FRotator::ZeroRotator;
		return;
	}
	
	TransformComponent.Location = StreamingSource->Location;
	TransformComponent.Rotation = StreamingSource->Rotation;
}
