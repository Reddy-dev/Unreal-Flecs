// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Viewers/Systems/FlecsViewerSyncSystems.h"

#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "SceneView.h"
#include "WorldPartition/WorldPartition.h"

#include "FlecsGameFrameworkModuleSettings.h"

#include "Viewers/Components/FlecsViewerPerspectiveComponent.h"
#include "Viewers/Components/FlecsViewerTransformComponent.h"
#include "Viewers/Components/FlecsViewerTypeComponents.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsViewerSyncSystems)

UFlecsPlayerViewerSyncSystem::UFlecsPlayerViewerSyncSystem()
{
}

void UFlecsPlayerViewerSyncSystem::BuildSystem(const TSolidNotNull<const UFlecsWorldInterfaceObject*> FlecsWorldInterfaceObject,
	TFlecsSystemBuilder<>& InBuilder) const
{
	InBuilder
		.Phase(EFlecsPhaseType::OnLoad)
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
	
	const APlayerController* PlayerController = PlayerComponent.PlayerController.Get();
	if UNLIKELY_IF(!::IsValid(PlayerController))
	{
		return;
	}
	
	if (bUsePlayerPawnLocationInsteadOfCamera)
	{
		if (const APawn* PlayerPawn = PlayerController->GetPawn())
		{
			TransformComponent.Location = PlayerPawn->GetActorLocation();
			TransformComponent.Rotation = PlayerPawn->GetActorRotation();
		}
		else
		{
			PlayerController->GetPlayerViewPoint(TransformComponent.Location, TransformComponent.Rotation);
		}
	}
	else
	{
		FVector PlayerCameraLocation(ForceInitToZero);
		FRotator PlayerCameraRotation(FRotator::ZeroRotator);
		
		PlayerController->GetPlayerViewPoint(PlayerCameraLocation, PlayerCameraRotation);
		
		TransformComponent.Location = PlayerCameraLocation;
		TransformComponent.Rotation = PlayerCameraRotation;
	}

	const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (LocalPlayer != nullptr && LocalPlayer->ViewportClient != nullptr && LocalPlayer->ViewportClient->Viewport != nullptr)
	{
		FSceneViewProjectionData ProjectionData;
		if (LocalPlayer->GetProjectionData(LocalPlayer->ViewportClient->Viewport, ProjectionData))
		{
			const FMatrix& ProjectionMatrix = ProjectionData.ProjectionMatrix;
			if (FMath::Abs(ProjectionMatrix.M[3][3]) < UE_KINDA_SMALL_NUMBER)
			{
				const double TanHalfHorizontal = FMath::Abs(ProjectionMatrix.M[0][0]) > UE_KINDA_SMALL_NUMBER
					? 1.0 / FMath::Abs(ProjectionMatrix.M[0][0])
					: 1.0;
				const double TanHalfVertical = FMath::Abs(ProjectionMatrix.M[1][1]) > UE_KINDA_SMALL_NUMBER
					? 1.0 / FMath::Abs(ProjectionMatrix.M[1][1])
					: 1.0;

				PerspectiveComponent.FieldOfView = static_cast<float>(FMath::RadiansToDegrees(FMath::Atan(TanHalfHorizontal) * 2.0));
				PerspectiveComponent.AspectRatio = static_cast<float>(TanHalfHorizontal / TanHalfVertical);
			}
		}
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
		.Phase(EFlecsPhaseType::OnLoad)
		.WithPair<FFlecsViewerRelationship, const FFlecsViewerActorComponent>() // 0
		.With<FFlecsViewerTransformComponent&>(); // 1
}

void UFlecsActorViewerSyncSystem::EachIterator(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld,
	flecs::iter& InIterator, const FFlecsId InIndex)
{
	const FFlecsViewerActorComponent& ActorComponent = InIterator.field_at<const FFlecsViewerActorComponent>(0, InIndex);
	FFlecsViewerTransformComponent& TransformComponent = InIterator.field_at<FFlecsViewerTransformComponent>(1, InIndex);
	const AActor* Actor = ActorComponent.Actor.Get();
	if UNLIKELY_IF(!::IsValid(Actor))
	{
		return;
	}
	
	TransformComponent.Location = Actor->GetActorLocation();
	TransformComponent.Rotation = Actor->GetActorRotation();
}

UFlecsStreamingSourceViewerSyncSystem::UFlecsStreamingSourceViewerSyncSystem()
{
}

void UFlecsStreamingSourceViewerSyncSystem::BuildSystem(const TSolidNotNull<const UFlecsWorldInterfaceObject*> FlecsWorldInterfaceObject,
	TFlecsSystemBuilder<>& InBuilder) const
{
	InBuilder
		.Phase(EFlecsPhaseType::OnLoad)
		.WithPair<FFlecsViewerRelationship, const FFlecsViewerStreamingSourceComponent>() // 0
		.With<FFlecsViewerTransformComponent&>(); // 1
}

void UFlecsStreamingSourceViewerSyncSystem::EachIterator(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld,
	flecs::iter& InIterator, const FFlecsId InIndex)
{
	const UWorldPartition* WorldPartition = InWorld->GetWorld()->GetWorldPartition();
	
	const FFlecsViewerStreamingSourceComponent& StreamingSourceComponent 
		= InIterator.field_at<const FFlecsViewerStreamingSourceComponent>(0, InIndex);
	FFlecsViewerTransformComponent& TransformComponent 
		= InIterator.field_at<FFlecsViewerTransformComponent>(1, InIndex);
	if UNLIKELY_IF(WorldPartition == nullptr)
	{
		TransformComponent.Location = FVector::ZeroVector;
		TransformComponent.Rotation = FRotator::ZeroRotator;
		return;
	}

	const TArray<FWorldPartitionStreamingSource>& StreamingSources = WorldPartition->GetStreamingSources();
	
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
