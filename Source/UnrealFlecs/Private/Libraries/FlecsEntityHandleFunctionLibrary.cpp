// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Libraries/FlecsEntityHandleFunctionLibrary.h"

#include "GameFramework/Actor.h"

#include "Worlds/FlecsWorldSubsystem.h"

#include "Interfaces/FlecsEntityInterface.h"

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

bool UFlecsEntityHandleFunctionLibrary::IsValid(const FFlecsEntityHandle& Entity)
{
	return Entity.IsValid();
}

void UFlecsEntityHandleFunctionLibrary::DestroyEntity(const FFlecsEntityHandle& Entity)
{
	Entity.Destroy();
}
