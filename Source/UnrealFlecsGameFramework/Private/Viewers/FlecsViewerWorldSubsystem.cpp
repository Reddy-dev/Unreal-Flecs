// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Viewers/FlecsViewerWorldSubsystem.h"

#include "Collections/FlecsCollectionWorldSubsystem.h"
#include "Viewers/Components/FlecsViewerCollectionTypes.h"
#include "Viewers/Components/FlecsViewerSubsystemSingleton.h"
#include "Viewers/Components/FlecsViewerTypeComponents.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsViewerWorldSubsystem)

void UFlecsViewerWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	
}

void UFlecsViewerWorldSubsystem::OnFlecsWorldInitialized(const TSolidNotNull<UFlecsWorld*> InWorld)
{
	InWorld->RegisterComponentType<FFlecsViewerSubsystemSingleton>();
	InWorld->Set<FFlecsViewerSubsystemSingleton>(FFlecsViewerSubsystemSingleton{ this });
	
	const TSolidNotNull<UFlecsCollectionWorldSubsystem*> CollectionSubsystem 
		= InWorld->GetWorld()->GetSubsystemChecked<UFlecsCollectionWorldSubsystem>();
	
	CollectionSubsystem->RegisterCollectionInterfaceClass<UFlecsPlayerViewerCollection>();
	CollectionSubsystem->RegisterCollectionInterfaceClass<UFlecsActorViewerCollection>();
	CollectionSubsystem->RegisterCollectionInterfaceClass<UFlecsStreamSourceViewerCollection>();
	
}

void UFlecsViewerWorldSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

FFlecsEntityHandle UFlecsViewerWorldSubsystem::CreatePlayerViewer(
	const TSolidNotNull<const APlayerController*> InPlayerController)
{
	return GetFlecsWorldChecked()->CreateEntity()
		.AddCollection<UFlecsPlayerViewerCollection, FFlecsViewerPlayerComponent>({InPlayerController});
}

FFlecsEntityHandle UFlecsViewerWorldSubsystem::AddActorViewer(const TSolidNotNull<const AActor*> InActor)
{
	return GetFlecsWorldChecked()->CreateEntity()
		.AddCollection<UFlecsActorViewerCollection, FFlecsViewerActorComponent>({InActor});
}

FFlecsEntityHandle UFlecsViewerWorldSubsystem::AddStreamSourceViewer(const FName& InStreamSourceName)
{
	return GetFlecsWorldChecked()->CreateEntity()
		.AddCollection<UFlecsStreamSourceViewerCollection, FFlecsViewerStreamingSourceComponent>({InStreamSourceName});
}
