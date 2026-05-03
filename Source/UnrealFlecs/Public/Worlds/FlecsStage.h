// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsWorldInterfaceObject.h"

#include "FlecsStage.generated.h"

// @TODO: Implement tracking stages properly
UCLASS(BlueprintType, NotBlueprintable)
class UNREALFLECS_API UFlecsStage final : public UFlecsWorldInterfaceObject
{
	GENERATED_BODY()

public:
	UFlecsStage(const FObjectInitializer& ObjectInitializer);
	
	virtual ~UFlecsStage() override;
	
	void DestroyStage();
	
	void SetStageWorld(const flecs::world& InStageWorld);
	
	UFUNCTION(BlueprintCallable, Category = "Flecs | Stage")
	int32 GetStageId() const;
	
	/**
	 * Merge world or stage.
	 * When automatic merging is disabled, an application can call this
	 * operation on either an individual stage, or on the world which will merge
	 * all stages. This operation may only be called when staging is not enabled
	 * (either after progress() or after readonly_end()).
	 *
	 * This operation may be called on an already merged stage or world.
	 *
	 * @see ecs_merge()
	 */
	UFUNCTION(BlueprintCallable, Category = "Flecs | Stage")
	void Merge() const;
	
protected:
	virtual flecs::world* GetNativeFlecsWorld_Internal() const override
	{
		return const_cast<flecs::world*>(&StageWorld);
	}
	
private:
	flecs::world StageWorld;
	
}; // class UFlecsStage
