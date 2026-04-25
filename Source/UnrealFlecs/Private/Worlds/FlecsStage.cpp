// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Worlds/FlecsStage.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsStage)

UFlecsStage::UFlecsStage(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UFlecsStage::~UFlecsStage()
{
	if (!IsValid(GetOuter()))
	{
		StageWorld.world_ = nullptr;
	}
}

void UFlecsStage::DestroyStage()
{
	StageWorld.release();
	MarkAsGarbage();
}

void UFlecsStage::SetStageWorld(const flecs::world& InStageWorld)
{
	solid_checkf(InStageWorld.is_stage(), TEXT("Provided world is not a stage"));
	StageWorld = InStageWorld;
}

int32 UFlecsStage::GetStageId() const
{
	return GetNativeFlecsWorld_Internal()->get_stage_id();
}

void UFlecsStage::Merge() const
{
	GetNativeFlecsWorld_Internal()->merge();
}
