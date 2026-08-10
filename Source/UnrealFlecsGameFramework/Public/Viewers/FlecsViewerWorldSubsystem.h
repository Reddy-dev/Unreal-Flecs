// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Worlds/FlecsAbstractWorldSubsystem.h"

#include "FlecsViewerWorldSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class UNREALFLECSGAMEFRAMEWORK_API UFlecsViewerWorldSubsystem : public UFlecsAbstractWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void OnFlecsWorldInitialized(const TSolidNotNull<UFlecsWorld*> InWorld) override;
	virtual void Deinitialize() override;
	
	FFlecsEntityHandle CreatePlayerViewer(const TSolidNotNull<const APlayerController*> InPlayerController);
	FFlecsEntityHandle AddActorViewer(const TSolidNotNull<const AActor*> InActor);
	FFlecsEntityHandle AddStreamSourceViewer(const FName& InStreamSourceName);
	

private:
	
	
}; // class UFlecsViewerWorldSubsystem
