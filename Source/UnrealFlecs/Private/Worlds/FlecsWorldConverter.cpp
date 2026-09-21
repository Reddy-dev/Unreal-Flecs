// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Worlds/FlecsWorldConverter.h"

#include "Worlds/FlecsWorldSubsystem.h"
#include "Worlds/FlecsWorld.h"
#include "Worlds/FlecsStage.h"
#include "Worlds/UnrealFlecsWorldTag.h"

TSolidNotNull<UFlecsWorldInterfaceObject*> UE::Flecs::ToUnrealFlecsWorldInterface(const flecs::world& InWorld)
{
	solid_checkf(InWorld, TEXT("Passed in flecs::world is not valid."));
	solid_checkf(InWorld.has<FUnrealFlecsWorldTag>(), TEXT("Passed in flecs::world is not an Unreal Flecs World."));
	
	if (!InWorld.is_stage())
	{
		return ToUnrealFlecsWorld(InWorld);
	}

	const flecs::world ActualWorld = InWorld.get_world();
	const TSolidNotNull<const UFlecsWorld*> FlecsWorld = ToUnrealFlecsWorld(ActualWorld);
	
	const int32 StageId = InWorld.get_stage_id();
	
	return FlecsWorld->GetStage(StageId);
}

TSolidNotNull<UFlecsWorld*> UE::Flecs::ToUnrealFlecsWorld(const flecs::world& InWorld)
{
	solid_checkf(InWorld, TEXT("Passed in flecs::world is not valid."));
	solid_checkf(InWorld.has<FUnrealFlecsWorldTag>(), TEXT("Passed in flecs::world is not an Unreal Flecs World."));
	
	solid_check(InWorld.get_ctx() != nullptr);

	const TSolidNotNull<UFlecsWorld*> FlecsWorld = static_cast<UFlecsWorldSubsystem*>(InWorld.get_ctx())->GetDefaultWorldChecked();
	solid_checkf(IsValid(FlecsWorld), TEXT("FlecsWorld is not valid."));
		
	return FlecsWorld;
}
