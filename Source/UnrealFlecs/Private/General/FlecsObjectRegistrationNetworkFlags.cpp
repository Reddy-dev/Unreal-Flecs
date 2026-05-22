// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "General/FlecsObjectRegistrationNetworkFlags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsObjectRegistrationNetworkFlags)

namespace UE::Flecs::Net
{
	EFlecsObjectRegistrationNetworkFlags GetFlagsForWorld(const TSolidNotNull<const UWorld*> InWorld)
	{
		switch (InWorld->GetNetMode())
		{
			case NM_Standalone:
				return EFlecsObjectRegistrationNetworkFlags::Standalone;
			case NM_DedicatedServer:
				return EFlecsObjectRegistrationNetworkFlags::Server;
			case NM_ListenServer:
				return EFlecsObjectRegistrationNetworkFlags::Server | EFlecsObjectRegistrationNetworkFlags::Client;
			case NM_Client:
				return EFlecsObjectRegistrationNetworkFlags::Client;
			default:
				checkf(false, TEXT("Invalid NetMode"));
				return EFlecsObjectRegistrationNetworkFlags::None;
		}
	}

	bool ShouldRegisterInWorld(const TSolidNotNull<const UWorld*> InWorld, const EFlecsObjectRegistrationNetworkFlags InFlags)
	{
		const EFlecsObjectRegistrationNetworkFlags WorldFlags = GetFlagsForWorld(InWorld);
		return (InFlags & WorldFlags) != EFlecsObjectRegistrationNetworkFlags::None;
	}
	
} // namespace UE::Flecs::Net
