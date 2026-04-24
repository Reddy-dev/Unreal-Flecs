// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FlecsGeneratorTermRef.h"

#include "StructUtils/InstancedStruct.h"

#include "SolidMacros/Macros.h"
#include "Types/SolidNotNull.h"

#include "FlecsQueryGeneratorInput.generated.h"

class UFlecsWorld;

struct FFlecsQueryGeneratorInputType;

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsQueryGeneratorInput
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bPair = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TInstancedStruct<FFlecsQueryGeneratorInputType> First;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TInstancedStruct<FFlecsQueryGeneratorInputType> Second;
	
	NO_DISCARD FORCEINLINE bool IsValid() const
	{
		return First.IsValid() && (!bPair || Second.IsValid());
	}
	
	NO_DISCARD FORCEINLINE bool IsPair() const
	{
		return bPair;
	}
	
	NO_DISCARD FFlecsTermRefAtom_Internal GetFirstTermRef(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld) const;
	NO_DISCARD FFlecsTermRefAtom_Internal GetSecondTermRef(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld) const;
	
private:
	
}; // struct FFlecsQueryGeneratorInput

