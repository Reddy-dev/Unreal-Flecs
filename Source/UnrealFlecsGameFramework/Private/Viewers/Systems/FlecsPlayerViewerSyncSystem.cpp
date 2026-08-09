// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Viewers/Systems/FlecsPlayerViewerSyncSystem.h"

#include "FlecsGameFrameworkModuleSettings.h"
#include "Viewers/Components/FlecsViewerPerspectiveComponent.h"
#include "Viewers/Components/FlecsViewerTransformComponent.h"
#include "Viewers/Components/FlecsViewerTypeComponents.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsPlayerViewerSyncSystem)

UFlecsPlayerViewerSyncSystem::UFlecsPlayerViewerSyncSystem()
{
}

void UFlecsPlayerViewerSyncSystem::BuildSystem(
	const TSolidNotNull<const UFlecsWorldInterfaceObject*> FlecsWorldInterfaceObject,
	TFlecsSystemBuilder<>& InBuilder) const
{
	InBuilder
		.Phase(EFlecsPhaseType::PreFrame)
		.WithPair<FFlecsViewerRelationship, const FFlecsViewerPlayerComponent>() // 0
		.With<FFlecsViewerTransformComponent&>() // 1
		.With<FFlecsViewerPerspectiveComponent&>(); // 2
}

void UFlecsPlayerViewerSyncSystem::EachIterator(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld,
	flecs::iter& InIterator, const FFlecsId InIndex)
{
	const TSolidNotNull<const UFlecsGameFrameworkModuleSettings*> GameFrameworkSettings = GetDefault<UFlecsGameFrameworkModuleSettings>();
	
	const FFlecsViewerPlayerComponent& PlayerComponent = InIterator.field_at<const FFlecsViewerPlayerComponent>(0, InIndex);
	
	FFlecsViewerTransformComponent& TransformComponent = InIterator.field_at<FFlecsViewerTransformComponent>(1, InIndex);
	FFlecsViewerPerspectiveComponent& PerspectiveComponent = InIterator.field_at<FFlecsViewerPerspectiveComponent>(2, InIndex);
	
	const TSolidNotNull<APlayerController*> PlayerController = PlayerComponent.PlayerController;
	
	
}
