﻿// Elie Wiese-Namir © 2025. All Rights Reserved.

// ReSharper disable CppExpressionWithoutSideEffects
#include "FlecsModuleInterface.h"

#include "Engine/World.h"

#include "Logs/FlecsCategories.h"

#include "Worlds/FlecsWorld.h"
#include "Worlds/FlecsWorldConverter.h"
#include "Worlds/FlecsWorldSubsystem.h"

#include "Components/FlecsModuleComponent.h"
#include "Components/ObjectTypes/FFlecsModuleComponentTag.h"
#include "Components/FlecsUObjectComponent.h"

#include "FlecsModuleInitEvent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsModuleInterface)

void IFlecsModuleInterface::ImportModule(const flecs::world& InWorld)
{
	World = Unreal::Flecs::ToFlecsWorld(InWorld);
	solid_check(IsWorldValid());

	const TSolidNotNull<UFlecsWorld*> FlecsWorld = this->World.Get();
	
	const TSolidNotNull<const UWorld*> GameWorld = FlecsWorld->GetWorld();
	solid_checkf(IsValid(GameWorld), TEXT("FlecsWorld's UWorld is invalid."));

	FlecsWorld->EndScope([this, &FlecsWorld]()
	{
		FlecsWorld->Defer([this, &FlecsWorld]()
		{
			ModuleEntity = FlecsWorld->CreateEntity(Execute_GetModuleName(_getUObject()))
				.Add(flecs::Module)
				.SetPairFirst<FFlecsUObjectComponent, FFlecsModuleComponentTag>(FFlecsUObjectComponent
				{
					_getUObject()
				})
				.Set<FFlecsModuleComponent>({ _getUObject()->GetClass() });
		});

		solid_check(ModuleEntity.IsValid());

		FlecsWorld->Scope(ModuleEntity, [this, FlecsWorld]()
		{
			InitializeModule(FlecsWorld, ModuleEntity);
			Execute_BP_InitializeModule(_getUObject(), FlecsWorld);
		});
	});

	FlecsWorld->Event<FFlecsModuleInitEvent>()
		.id<FFlecsModuleComponent>()
		.entity(ModuleEntity)
		.emit();

	const TSolidNotNull<UFlecsWorldSubsystem*> WorldSubsystem = GameWorld->GetSubsystemChecked<UFlecsWorldSubsystem>();

	WorldSubsystem->ListenBeginPlay(
		FFlecsOnWorldBeginPlay::FDelegate::CreateLambda(
		[this, FlecsWorld](TSolidNotNull<UWorld*> InGameWorld)
		{
			WorldBeginPlay(FlecsWorld, InGameWorld);
			Execute_BP_WorldBeginPlay(_getUObject(), FlecsWorld, InGameWorld);
		}));
	
	UE_LOGFMT(LogFlecsCore, Verbose,
		"Imported module: {ModuleName}", IFlecsModuleInterface::Execute_GetModuleName(_getUObject()));
}

void IFlecsModuleInterface::DeinitializeModule_Internal()
{
	ModuleEntity.Disable();

	if (World.IsValid())
	{
		const TSolidNotNull<UFlecsWorld*> FlecsWorld = World.Get();
		
		DeinitializeModule(FlecsWorld);
		Execute_BP_DeinitializeModule(_getUObject(), FlecsWorld);
	}

	UE_LOGFMT(LogFlecsCore, Log,
		"Deinitialized module: {ModuleName}", IFlecsModuleInterface::Execute_GetModuleName(_getUObject()));
}

void IFlecsModuleInterface::InitializeModule(TSolidNotNull<UFlecsWorld*> InWorld, const FFlecsEntityHandle& InModuleEntity)
{
}

void IFlecsModuleInterface::WorldBeginPlay(TSolidNotNull<UFlecsWorld*> InWorld, TSolidNotNull<UWorld*> InGameWorld)
{
}

void IFlecsModuleInterface::DeinitializeModule(TSolidNotNull<UFlecsWorld*> InWorld)
{
}

FString IFlecsModuleInterface::GetModuleName_Implementation() const
{
	UE_LOGFMT(LogFlecsCore, Warning,
		"Module name not implemented! It is recommended to implement this function in the module interface. "
		"Will return the inherited class name instead. For Class: {ClassName}", _getUObject()->GetClass()->GetName());
	
	return _getUObject()->GetClass()->GetName();
}
