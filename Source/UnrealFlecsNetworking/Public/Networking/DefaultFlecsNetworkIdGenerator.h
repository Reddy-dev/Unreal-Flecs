// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once


#include "UObject/Object.h"

#include "FlecsNetworkIDGeneratorInterface.h"

#include "DefaultFlecsNetworkIdGenerator.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class UNREALFLECSNETWORKING_API UDefaultFlecsNetworkIdGenerator : public UObject, public IFlecsNetworkIDGeneratorInterface
{
	GENERATED_BODY()

public:
	virtual FFlecsNetworkId GenerateNetworkId() override;
	virtual bool ReleaseNetworkId(const FFlecsNetworkId& NetworkId) override;
	virtual void ResetNetworkIdGenerator() override;
	
protected:
	UPROPERTY()
	TArray<uint32> FreeSlotIds;
	
	UPROPERTY()
	TArray<uint32> Generations;
	
}; // class UDefaultFlecsNetworkIdGenerator
