// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/DefaultFlecsNetworkIdGenerator.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DefaultFlecsNetworkIdGenerator)

FFlecsNetworkId UDefaultFlecsNetworkIdGenerator::GenerateNetworkId()
{
	uint32 Slot = 0;
	
	if (!FreeSlotIds.IsEmpty())
	{
		Slot = FreeSlotIds.Pop();
		Generations[Slot]++;
		return FFlecsNetworkId(Slot, Generations[Slot]);
	}
	else
	{
		Slot = Generations.Add(1);
		return FFlecsNetworkId(Slot, 1);
	}
}

bool UDefaultFlecsNetworkIdGenerator::ReleaseNetworkId(const FFlecsNetworkId& NetworkId)
{
	if (NetworkId.IsValid())
	{
		FreeSlotIds.Add(NetworkId.GetSlot());
		return true;
	}
	
	return false;
}

void UDefaultFlecsNetworkIdGenerator::ResetNetworkIdGenerator()
{
	FreeSlotIds.Empty();
	Generations.Empty();
}
