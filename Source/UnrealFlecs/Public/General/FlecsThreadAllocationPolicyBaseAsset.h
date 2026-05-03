// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Types/SolidNotNull.h"

#include "FlecsThreadAllocationPolicyBaseAsset.generated.h"

class UFlecsWorld;

UENUM(BlueprintType)
enum class EFlecsThreadAllocationType : uint8
{
	RunnableThreads,
	TaskThreads
}; // enum class EFlecsThreadAllocationType

/**
 * 
 */
UCLASS(Abstract, BlueprintType, NotBlueprintable)
class UNREALFLECS_API UFlecsThreadAllocationPolicyBaseAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual TTuple<EFlecsThreadAllocationType, int32> GetThreadCountAllocation(const TSolidNotNull<const UFlecsWorld*> InWorld) const
		PURE_VIRTUAL(UFlecsThreadAllocationPolicyBase::GetThreadAllocationForSystem, return TTuple<EFlecsThreadAllocationType, uint32>(EFlecsThreadAllocationType::RunnableThreads, 1););
	
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	
}; // class UFlecsThreadAllocationPolicyBaseAsset
