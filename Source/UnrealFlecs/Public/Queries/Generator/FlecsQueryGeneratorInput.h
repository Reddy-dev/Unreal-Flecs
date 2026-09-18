// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once


#include "StructUtils/InstancedStruct.h"

#include "SolidMacros/Macros.h"
#include "Types/SolidNotNull.h"

#include "FlecsGeneratorTermRef.h"
#include "FlecsQueryGeneratorInputType.h"

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
	
	template <bool bAllowAllocation>
	NO_DISCARD FFlecsTermRefAtom_Internal GetFirstTermRef(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld) const;
	
	template <bool bAllowAllocation>
	NO_DISCARD FFlecsTermRefAtom_Internal GetSecondTermRef(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld) const;
	
private:
	
}; // struct FFlecsQueryGeneratorInput

template <bool bAllowAllocation>
FFlecsTermRefAtom_Internal FFlecsQueryGeneratorInput::GetFirstTermRef(
	const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld) const
{
	solid_checkf(First.IsValid(), TEXT("First term ref must be valid."));

	const FFlecsTermRef TermRef = First.Get().GetTermRefOutput(InWorld);
	
	return UE::Flecs::Queries::ToTermRefAtom<bAllowAllocation>(TermRef);
}

template <bool bAllowAllocation>
FFlecsTermRefAtom_Internal FFlecsQueryGeneratorInput::GetSecondTermRef(
	const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld) const
{
	solid_checkf(Second.IsValid(), TEXT("Second term ref must be valid."));

	const FFlecsTermRef TermRef = Second.Get().GetTermRefOutput(InWorld);
	
	return UE::Flecs::Queries::ToTermRefAtom<bAllowAllocation>(TermRef);
}
