// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Observers/FlecsObserverObject.h"

#include "Components/FlecsUObjectComponent.h"
#include "Components/ObjectTypes/FFlecsUObjectTag.h"
#include "Worlds/FlecsWorldInterfaceObject.h"
#include "Worlds/FlecsWorld.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsObserverObject)

UFlecsObserverObject::UFlecsObserverObject()
{
}

UFlecsObserverObject::UFlecsObserverObject(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlecsObserverObject::BuildObserver(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld,
	TFlecsObserverBuilder<>& InOutBuilder) const
{
	
}

UFlecsWorld* UFlecsObserverObject::GetFlecsWorld() const
{
	solid_checkf(GetOuter(), TEXT("FlecsObserverObject '%s' is not contained within a UFlecsWorld."), *GetName());
	
	return GetTypedOuter<UFlecsWorld>();
}

void UFlecsObserverObject::RegisterObject(const TSolidNotNull<UFlecsWorldInterfaceObject*> InFlecsWorld)
{
}

void UFlecsObserverObject::UnregisterObject(const TSolidNotNull<UFlecsWorldInterfaceObject*> InFlecsWorld)
{
	if LIKELY_IF(ObserverHandle.IsValid())
	{
		ObserverHandle.Destroy();
		ObserverHandle.ResetHandle();
	}
}

void UFlecsObserverObject::FlecsWorldBeginPlay(const TSolidNotNull<UFlecsWorldInterfaceObject*> InFlecsWorld)
{
	InitializeObserver(InFlecsWorld);
}

void UFlecsObserverObject::InitializeObserver(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld)
{
	const FString ObserverName = GetName();
	
	TFlecsObserverBuilder<> ObserverBuilder = InWorld->CreateObserverWithDefinition(ObserverDefinition, ObserverName);
	BuildObserver(InWorld, ObserverBuilder);
	
	ObserverHandle = ObserverBuilder.run([this](flecs::iter& InIter)
	{
		this->RunIterator(GetFlecsWorld(), InIter);
	});
	
	ObserverHandle.SetPair<FFlecsUObjectComponent, FFlecsUObjectTag>(FFlecsUObjectComponent(this));
}
