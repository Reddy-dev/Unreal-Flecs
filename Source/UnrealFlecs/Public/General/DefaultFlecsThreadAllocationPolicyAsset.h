// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsThreadAllocationPolicyBaseAsset.h"

#include "DefaultFlecsThreadAllocationPolicyAsset.generated.h"

/**
 * 
 */
UCLASS()
class UNREALFLECS_API UDefaultFlecsThreadAllocationPolicyAsset : public UFlecsThreadAllocationPolicyBaseAsset
{
	GENERATED_BODY()

public:
	virtual TTuple<EFlecsThreadAllocationType, int32> GetThreadCountAllocation(
		const TSolidNotNull<const UFlecsWorld*> InWorld) const override;
	
	UPROPERTY(EditAnywhere, Category = "Flecs | Thread Allocation")
	EFlecsThreadAllocationType ThreadAllocationType = EFlecsThreadAllocationType::RunnableThreads;
	
	UPROPERTY(EditAnywhere, Category = "Flecs | Thread Allocation")
	uint32 ThreadCount = 8;
	
	
}; // class UDefaultFlecsThreadAllocationPolicy
