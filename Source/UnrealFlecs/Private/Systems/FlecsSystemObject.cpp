// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Systems/FlecsSystemObject.h"

#include "Components/FlecsUObjectComponent.h"
#include "Components/ObjectTypes/FFlecsUObjectTag.h"
#include "Worlds/FlecsStage.h"
#include "Worlds/FlecsWorld.h"
#include "Worlds/FlecsWorldConverter.h"

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

void UFlecsSystemObject::EnableSystem() const
{
	GetSystemHandle().Enable();
}

void UFlecsSystemObject::DisableSystem() const
{
	GetSystemHandle().Disable();
}

bool UFlecsSystemObject::IsSystemEnabled() const
{
	return GetSystemHandle().IsEnabled();
}

void UFlecsSystemObject::SetContext(void* InContext) const
{
	GetSystemHandle().SetContext(InContext);
}

void UFlecsSystemObject::ApplySystemDefinitionOverrides(FFlecsSystemDefinition& InOutDefinition) const
{
	if (SystemDefinitionOverrides.PhaseInputOverride.IsSet())
	{
		InOutDefinition.PhaseInput = SystemDefinitionOverrides.PhaseInputOverride.GetValue();
	}
	
	if (SystemDefinitionOverrides.IntervalOverride.IsSet())
	{
		InOutDefinition.Interval = SystemDefinitionOverrides.IntervalOverride.GetValue();
	}
	
	if (SystemDefinitionOverrides.RateOverride.IsSet())
	{
		InOutDefinition.Rate = SystemDefinitionOverrides.RateOverride.GetValue();
	}
	
	if (SystemDefinitionOverrides.TickSourceInputOverride.IsSet())
	{
		InOutDefinition.TickSourceInput = SystemDefinitionOverrides.TickSourceInputOverride.GetValue();
	}
	
	if (SystemDefinitionOverrides.MultiThreadedOverride.IsSet())
	{
		InOutDefinition.bMultiThreaded = SystemDefinitionOverrides.MultiThreadedOverride.GetValue();
	}
	
	if (SystemDefinitionOverrides.ImmediateOverride.IsSet())
	{
		InOutDefinition.bImmediate = SystemDefinitionOverrides.ImmediateOverride.GetValue();
	}
	
	if (SystemDefinitionOverrides.PipelineInputOverride.IsSet())
	{
		InOutDefinition.PipelineInput = SystemDefinitionOverrides.PipelineInputOverride.GetValue();
	}
}

void UFlecsSystemObject::OnBuildSystem(const FFlecsSystemHandle& InSystemHandle)
{
	InSystemHandle.SetPair<FFlecsUObjectComponent, FFlecsUObjectTag>(FFlecsUObjectComponent(this));
	
	if (bStartsDisabled)
	{
		DisableSystem();
	}
}

void UFlecsSystemObject::InitializeSystem(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld)
{
	const FFlecsEntityHandle ScriptClassType = InWorld->RegisterScriptClassType(this->GetClass());
	
	TFlecsSystemBuilder<> SystemBuilder = InWorld->CreateSystemWithDefinition(SystemDefinition, ScriptClassType.GetFlecsId());
	BuildSystem(InWorld, SystemBuilder);
	ApplySystemDefinitionOverrides(SystemBuilder.GetSystemDefinition());
	
	SystemHandle = SystemBuilder.run([this](flecs::iter& InIterator)
	{
		const TSolidNotNull<UFlecsWorldInterfaceObject*> IteratorWorld = UE::Flecs::ToUnrealFlecsWorldInterface(InIterator.world());
		this->RunIterator(IteratorWorld, InIterator);
	});
	
	OnBuildSystem(SystemHandle);
}
