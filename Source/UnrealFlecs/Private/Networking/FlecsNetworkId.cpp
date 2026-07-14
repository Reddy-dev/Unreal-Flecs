// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/FlecsNetworkId.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsNetworkId)

REGISTER_FLECS_COMPONENT(FFlecsNetworkId);

FFlecsNetworkIdAllocator::FFlecsNetworkIdAllocator(const uint8 InSessionEpoch)
{
	Reset(InSessionEpoch);
}

FFlecsNetworkId FFlecsNetworkIdAllocator::Allocate()
{
	uint32 Slot = 0;
	if (!FreeSlots.IsEmpty())
	{
		Slot = FreeSlots.Pop(EAllowShrinking::No);
	}
	else
	{
		Slot = NextSlot++;
		if (Slot == 0)
		{
			return {};
		}
	}

	uint32& Generation = SlotGenerations.FindOrAdd(Slot, 1);
	if (Generation == 0)
	{
		Generation = 1;
	}
	
	AllocatedSlots.Add(Slot);
	return FFlecsNetworkId(Slot, Generation, SessionEpoch);
}

bool FFlecsNetworkIdAllocator::Release(const FFlecsNetworkId InId)
{
	if (!InId.IsValid() || InId.GetSessionEpoch() != SessionEpoch || !AllocatedSlots.Remove(InId.GetSlot()))
	{
		return false;
	}

	uint32& Generation = SlotGenerations.FindChecked(InId.GetSlot());
	++Generation;
	if (Generation > FFlecsNetworkId::GenerationValueMask)
	{
		Generation = 1;
	}
	FreeSlots.Add(InId.GetSlot());
	return true;
}

void FFlecsNetworkIdAllocator::Reset(const uint8 InSessionEpoch)
{
	SessionEpoch = InSessionEpoch == 0 ? 1 : InSessionEpoch;
	NextSlot = 1;
	FreeSlots.Reset();
	SlotGenerations.Reset();
	AllocatedSlots.Reset();
}
