// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Systems/FlecsSystemObject.h"

#include "Worlds/FlecsWorld.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsSystemObject)

UFlecsSystemObject::UFlecsSystemObject()
{
}

UFlecsSystemObject::UFlecsSystemObject(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlecsSystemObject::BuildSystem(const TSolidNotNull<const UFlecsWorld*> InWorld,
	TFlecsSystemBuilder<>& InBuilder) const
{
}

UFlecsWorld* UFlecsSystemObject::GetFlecsWorld() const
{
	solid_checkf(GetOuter(), TEXT("FlecsObserverObject '%s' is not contained within a UFlecsWorld."), *GetName());
	
	return GetTypedOuter<UFlecsWorld>();
}

void UFlecsSystemObject::RegisterObject(const TSolidNotNull<UFlecsWorld*> InFlecsWorld)
{
}

void UFlecsSystemObject::UnregisterObject(const TSolidNotNull<UFlecsWorld*> InFlecsWorld)
{
	if LIKELY_IF(SystemHandle.IsValid())
	{
		SystemHandle.Destroy();
		SystemHandle.ResetHandle();
	}
}

void UFlecsSystemObject::FlecsWorldBeginPlay(const TSolidNotNull<UFlecsWorld*> InFlecsWorld)
{
	InitializeSystem(InFlecsWorld);
}

void UFlecsSystemObject::InitializeSystem(const TSolidNotNull<const UFlecsWorld*> InWorld)
{
	const FString SystemName = GetName();
	
	TFlecsSystemBuilder<> SystemBuilder = InWorld->CreateSystemWithDefinition(SystemDefinition, SystemName);
	BuildSystem(InWorld, SystemBuilder);
	
	SystemHandle = SystemBuilder.run([this](flecs::iter& InIterator)
	{
		this->RunIterator(GetFlecsWorld(), InIterator);
	});
}
