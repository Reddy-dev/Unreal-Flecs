// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "SolidTestGuardTypes.h"

#include "SolidMacros/Macros.h"

Solid::Tests::FNetDriverTickRateAdjuster::FNetDriverTickRateAdjuster()
{
	Handle = FWorldDelegates::OnNetDriverCreated.AddRaw(this, &FNetDriverTickRateAdjuster::OnNetDriverCreated);
}

Solid::Tests::FNetDriverTickRateAdjuster::~FNetDriverTickRateAdjuster()
{
	FWorldDelegates::OnNetDriverCreated.Remove(Handle);
}

void Solid::Tests::FNetDriverTickRateAdjuster::OnNetDriverCreated(MAYBE_UNUSED UWorld* InWorld, UNetDriver* InNetDriver)
{
	solid_cassume(InNetDriver);
		
	// Make sure NetDriver will tick every engine frame
	InNetDriver->MaxNetTickRate = 0;
}
