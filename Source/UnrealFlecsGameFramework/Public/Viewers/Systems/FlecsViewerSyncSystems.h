// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Systems/FlecsSystemObject.h"

#include "FlecsViewerSyncSystems.generated.h"

/**
 * 
 */
UCLASS()
class UNREALFLECSGAMEFRAMEWORK_API UFlecsPlayerViewerSyncSystem : public UFlecsSystemObject
{
	GENERATED_BODY()

public:
	UFlecsPlayerViewerSyncSystem();
	
	virtual void BuildSystem(const TSolidNotNull<const UFlecsWorldInterfaceObject*>, TFlecsSystemBuilder<>& InBuilder) const override;
	virtual void EachIterator(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld, flecs::iter& InIterator, const FFlecsId InIndex) override;
	
}; // class UFlecsPlayerViewerSyncSystem

UCLASS()
class UNREALFLECSGAMEFRAMEWORK_API UFlecsActorViewerSyncSystem : public UFlecsSystemObject
{
	GENERATED_BODY()
	
public:
	UFlecsActorViewerSyncSystem();
	
	virtual void BuildSystem(const TSolidNotNull<const UFlecsWorldInterfaceObject*>, TFlecsSystemBuilder<>& InBuilder) const override;
	virtual void EachIterator(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld, flecs::iter& InIterator, const FFlecsId InIndex) override;
}; // class UFlecsActorViewerSyncSystem

UCLASS()
class UNREALFLECSGAMEFRAMEWORK_API UFlecsStreamingSourceViewerSyncSystem : public UFlecsSystemObject
{
	GENERATED_BODY()
	
public:
	UFlecsStreamingSourceViewerSyncSystem();
	
	virtual void BuildSystem(const TSolidNotNull<const UFlecsWorldInterfaceObject*>, TFlecsSystemBuilder<>& InBuilder) const override;
	virtual void EachIterator(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld, flecs::iter& InIterator, const FFlecsId InIndex) override;
	
}; // class UFlecsStreamingSourceViewerSyncSystem
