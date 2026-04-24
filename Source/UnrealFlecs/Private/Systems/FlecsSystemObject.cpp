// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Systems/FlecsSystemObject.h"

#include "Components/FlecsUObjectComponent.h"
#include "Components/ObjectTypes/FFlecsUObjectTag.h"
#include "Worlds/FlecsStage.h"
#include "Worlds/FlecsWorld.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsSystemObject)

UFlecsSystemObject::UFlecsSystemObject()
{
}

UFlecsSystemObject::UFlecsSystemObject(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlecsSystemObject::BuildSystem(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld,
	TFlecsSystemBuilder<>& InBuilder) const
{
}

UFlecsWorld* UFlecsSystemObject::GetFlecsWorld() const
{
	solid_checkf(GetOuter(), TEXT("FlecsObserverObject '%s' is not contained within a UFlecsWorld."), *GetName());
	
	return GetTypedOuter<UFlecsWorld>();
}

void UFlecsSystemObject::RegisterObject(const TSolidNotNull<UFlecsWorldInterfaceObject*> InFlecsWorld)
{
}

void UFlecsSystemObject::UnregisterObject(const TSolidNotNull<UFlecsWorldInterfaceObject*> InFlecsWorld)
{
	if LIKELY_IF(SystemHandle.IsValid())
	{
		SystemHandle.Destroy();
		SystemHandle.ResetHandle();
	}
}

void UFlecsSystemObject::FlecsWorldBeginPlay(const TSolidNotNull<UFlecsWorldInterfaceObject*> InFlecsWorld)
{
	InitializeSystem(InFlecsWorld);
}

void UFlecsSystemObject::RunSystem(const double InDeltaTime, void* InParams) const
{
	GetSystemHandle().Run(InDeltaTime, InParams);
}

void UFlecsSystemObject::SetContext(void* InContext) const
{
	GetSystemHandle().SetContext(InContext);
}

void UFlecsSystemObject::InitializeSystem(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld)
{
	const FString SystemName = GetName();
	
	TFlecsSystemBuilder<> SystemBuilder = InWorld->CreateSystemWithDefinition(SystemDefinition, SystemName);
	BuildSystem(InWorld, SystemBuilder);
	
	SystemHandle = SystemBuilder.run([this](flecs::iter& InIterator)
	{
		UFlecsWorldInterfaceObject* IteratorWorld = nullptr;
		
		if (InIterator.world().is_stage())
		{
			IteratorWorld = GetFlecsWorld()->GetStage(InIterator.world().get_stage_id());
		}
		else
		{
			IteratorWorld = GetFlecsWorld();
		}
		
		this->RunIterator(IteratorWorld, InIterator);
	});
	
	SystemHandle.SetPair<FFlecsUObjectComponent, FFlecsUObjectTag>(FFlecsUObjectComponent(this));
}
