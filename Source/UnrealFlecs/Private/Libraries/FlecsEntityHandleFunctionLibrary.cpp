// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Libraries/FlecsEntityHandleFunctionLibrary.h"

#include "GameFramework/Actor.h"

#include "Worlds/FlecsWorldSubsystem.h"

#include "Interfaces/FlecsEntityInterface.h"
#include "Worlds/FlecsWorldInterfaceObject.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsEntityHandleFunctionLibrary)

/*bool UEntityFunctionLibrary::HasEntityFromObject(UObject* Object)
{
	if UNLIKELY_IF(!ensureMsgf(Object, TEXT("Object is not valid")))
	{
		return false;
	}

	if (Object->Implements<UFlecsEntityInterface>())
	{
		return true;
	}

	if (const AActor* Actor = Cast<AActor>(Object))
	{
		return Actor->FindComponentByClass<UFlecsEntityActorComponent>() != nullptr;
	}
        
	return false;
}

FFlecsEntityHandle UEntityFunctionLibrary::GetEntityFromObject(UObject* Object)
{
	solid_check(Object);

	if (Object->Implements<UFlecsEntityInterface>())
	{
		return CastChecked<IFlecsEntityInterface>(Object)->GetEntityHandle();
	}

	if (const AActor* Actor = Cast<AActor>(Object))
	{
		if (const UFlecsEntityActorComponent* EntityActorComponent = Actor->FindComponentByClass<UFlecsEntityActorComponent>())
		{
			return EntityActorComponent->GetEntityHandle();
		}
	}

	return FFlecsEntityHandle::GetNullHandle();
}*/

bool UFlecsEntityHandleFunctionLibrary::ToBool_FlecsEntityView(const FFlecsEntityView& Id)
{
	return Id;
}

bool UFlecsEntityHandleFunctionLibrary::IsValid_FlecsEntityView(const FFlecsEntityView& Id)
{
	return Id.IsValid();
}

bool UFlecsEntityHandleFunctionLibrary::IsAlive_FlecsEntityView(const FFlecsEntityView& Id)
{
	return Id.IsAlive();
}

FFlecsId UFlecsEntityHandleFunctionLibrary::ToFlecsId_FlecsEntityView(const FFlecsEntityView& Id)
{
	return Id.GetFlecsId();
}

bool UFlecsEntityHandleFunctionLibrary::IsUnrealFlecsWorld(const FFlecsEntityView& Id)
{
	return Id.IsUnrealFlecsWorld();
}

bool UFlecsEntityHandleFunctionLibrary::IsInStage(const FFlecsEntityView& Id)
{
	return Id.IsInStage();
}

FFlecsId UFlecsEntityHandleFunctionLibrary::GetFlecsId(const FFlecsEntityView& Entity)
{
	return Entity.GetFlecsId();
}

UFlecsWorldInterfaceObject* UFlecsEntityHandleFunctionLibrary::GetFlecsWorld(const FFlecsEntityView& Entity)
{
	return Entity.GetFlecsWorld();
}

UWorld* UFlecsEntityHandleFunctionLibrary::GetOuterWorld(const FFlecsEntityView& Entity)
{
	return Entity.GetOuterWorld();
}

FString UFlecsEntityHandleFunctionLibrary::ToString_FlecsEntityView(const FFlecsEntityView& Id)
{
	return Id.ToString();
}

bool UFlecsEntityHandleFunctionLibrary::EqualEqual_FlecsEntityView(const FFlecsEntityView& A, const FFlecsEntityView& B)
{
	return A == B;
}

bool UFlecsEntityHandleFunctionLibrary::NotEqual_FlecsEntityView(const FFlecsEntityView& A, const FFlecsEntityView& B)
{
	return A != B;
}

bool UFlecsEntityHandleFunctionLibrary::EqualEqual_FlecsEntityHandle(const FFlecsEntityHandle& A,
	const FFlecsEntityHandle& B)
{
	return A == B;
}

bool UFlecsEntityHandleFunctionLibrary::NotEqual_FlecsEntityHandle(const FFlecsEntityHandle& A,
	const FFlecsEntityHandle& B)
{
	return A != B;
}

FFlecsEntityView UFlecsEntityHandleFunctionLibrary::Conv_FlecsEntityHandleToView(const FFlecsEntityHandle& Entity)
{
	return FFlecsEntityView(Entity);
}

FFlecsId UFlecsEntityHandleFunctionLibrary::Conv_FlecsEntityViewToId(const FFlecsEntityView& Entity)
{
	return Entity;
}

FFlecsId UFlecsEntityHandleFunctionLibrary::Conv_FlecsEntityHandleToId(const FFlecsEntityHandle& Entity)
{
	return Entity;
}

FFlecsEntityHandle UFlecsEntityHandleFunctionLibrary::ToMut_FlecsEntityViewWithWorld(const FFlecsEntityView& Entity,
                                                                                     const UFlecsWorldInterfaceObject* World)
{
	solid_cassumef(World, TEXT("World is not valid"));
	return Entity.ToMut<FFlecsEntityHandle>(World->GetNativeFlecsWorld());
}

bool UFlecsEntityHandleFunctionLibrary::IsComponent(const FFlecsEntityView& Entity)
{
	return Entity.IsComponent();
}

bool UFlecsEntityHandleFunctionLibrary::IsTag(const FFlecsEntityView& Entity)
{
	return Entity.IsTag();
}

bool UFlecsEntityHandleFunctionLibrary::IsValueComponent(const FFlecsEntityView& Entity)
{
	return Entity.IsValueComponent();
}

bool UFlecsEntityHandleFunctionLibrary::IsEnum(const FFlecsEntityView& Entity)
{
	return Entity.IsEnum();
}

bool UFlecsEntityHandleFunctionLibrary::HasParent(const FFlecsEntityView& Entity)
{
	return Entity.HasParent();
}

FFlecsEntityHandle UFlecsEntityHandleFunctionLibrary::GetParent(const FFlecsEntityView& Entity)
{
	return Entity.GetParent<FFlecsEntityHandle>();
}

bool UFlecsEntityHandleFunctionLibrary::IsPrefab(const FFlecsEntityView& Entity)
{
	return Entity.IsPrefab();
}

bool UFlecsEntityHandleFunctionLibrary::IsEnabled(const FFlecsEntityView& Entity)
{
	return Entity.IsEnabled();
}

bool UFlecsEntityHandleFunctionLibrary::HasName(const FFlecsEntityView& Entity)
{
	return Entity.HasName();
}

FString UFlecsEntityHandleFunctionLibrary::GetName(const FFlecsEntityView& Entity)
{
	return Entity.GetName();
}

FString UFlecsEntityHandleFunctionLibrary::GetPath(const FFlecsEntityView& Entity, const FString& Separator,
	const FString& RootSeparator)
{
	return Entity.GetPath(StringCast<char>(*Separator).Get(), StringCast<char>(*RootSeparator).Get());
}

void UFlecsEntityHandleFunctionLibrary::DestroyEntity(const FFlecsEntityHandle& Entity)
{
	Entity.Destroy();
}

void UFlecsEntityHandleFunctionLibrary::ClearEntity(const FFlecsEntityHandle& Entity)
{
	Entity.Clear();
}

void UFlecsEntityHandleFunctionLibrary::EnableEntity(const FFlecsEntityHandle& Entity)
{
	Entity.Enable();
}

void UFlecsEntityHandleFunctionLibrary::DisableEntity(const FFlecsEntityHandle& Entity)
{
	Entity.Disable();
}

bool UFlecsEntityHandleFunctionLibrary::ToggleEntity(const FFlecsEntityHandle& Entity)
{
	return Entity.Toggle();
}

void UFlecsEntityHandleFunctionLibrary::SetName(const FFlecsEntityHandle& Entity, const FString& Name)
{
	Entity.SetName(Name);
}

void UFlecsEntityHandleFunctionLibrary::SetParent(const FFlecsEntityHandle& Entity, const FFlecsEntityHandle& Parent,
	const bool bDontFragment)
{
	if (bDontFragment)
	{
		Entity.SetParent(Parent);
	}
	else
	{
		Entity.SetChildOf(Parent);
	}
}
