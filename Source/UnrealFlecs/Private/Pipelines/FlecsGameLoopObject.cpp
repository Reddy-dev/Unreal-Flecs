// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Pipelines/FlecsGameLoopObject.h"

#include "Engine/World.h"
#include "Worlds/FlecsWorld.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsGameLoopObject)

IMPLEMENT_SOLID_ASSET_VERSION(UFlecsGameLoopObject, 0x7C418350, 0x1DE64E39, 0x83C28526, 0x98DB6D7C, "FlecsGameLoopObjectAssetVersion");

UFlecsGameLoopObject::UFlecsGameLoopObject()
{
}

UFlecsGameLoopObject::UFlecsGameLoopObject(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlecsGameLoopObject::DeinitializeGameLoop(TSolidNotNull<UFlecsWorld*> InWorld, const FFlecsEntityHandle& InGameLoopEntity)
{
	IFlecsGameLoopInterface::DeinitializeGameLoop(InWorld, InGameLoopEntity);
	
	GetTickFunction().Get<>().UnRegisterTickFunction();
}

TSharedStruct<FFlecsTickFunction> UFlecsGameLoopObject::InitializeTickFunction(TSolidNotNull<UFlecsWorld*> InWorld)
{
	TickFunction = FFlecsTickFunctionSettingsInfo::CreateTickFunctionInstance(TickFunctionSettings);
	
	TickFunction.Get<>().OwningGameLoop = this;
	TickFunction.Get<>().OwningWorld = InWorld;
	TickFunction.Get<>().RegisterTickFunction(InWorld->GetWorld()->PersistentLevel);
	
	return TickFunction;
}

TSharedStruct<FFlecsTickFunction> UFlecsGameLoopObject::GetTickFunction() const
{
	return TickFunction;
}
