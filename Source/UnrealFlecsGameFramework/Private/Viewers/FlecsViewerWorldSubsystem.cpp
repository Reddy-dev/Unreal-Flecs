// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Viewers/FlecsViewerWorldSubsystem.h"

#include "Viewers/Components/FlecsViewerSubsystemSingleton.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsViewerWorldSubsystem)

void UFlecsViewerWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	
}

void UFlecsViewerWorldSubsystem::OnFlecsWorldInitialized(const TSolidNotNull<UFlecsWorld*> InWorld)
{
	InWorld->RegisterComponentType<FFlecsViewerSubsystemSingleton>();
	InWorld->Set<FFlecsViewerSubsystemSingleton>(FFlecsViewerSubsystemSingleton{ this });
}

void UFlecsViewerWorldSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

FFlecsEntityHandle UFlecsViewerWorldSubsystem::CreatePlayerViewer(
	const TSolidNotNull<APlayerController*> InPlayerController)
{
}

FFlecsEntityHandle UFlecsViewerWorldSubsystem::AddActorViewer(const TSolidNotNull<AActor*> InActor)
{
	
}

FFlecsEntityHandle UFlecsViewerWorldSubsystem::AddStreamSourceViewer(const FName& InStreamSourceName)
{
}
