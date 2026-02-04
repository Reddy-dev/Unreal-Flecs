// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Properties/FlecsComponentProperties.h"

#include "FlecsSubEntityRecordNameComponent.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsSubEntityRecordNameComponent
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = "Flecs")
	FString SubEntityName;
	
}; // struct FFlecsSubEntityRecordNameComponent

REGISTER_FLECS_COMPONENT(FFlecsSubEntityRecordNameComponent,
	[](flecs::world InWorld, const FFlecsComponentHandle& InComponentHandle)
	{
		InComponentHandle
			//.Add(flecs::DontFragment)
			.AddPair(flecs::OnInstantiate, flecs::Override);
	});