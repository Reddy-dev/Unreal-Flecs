// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Properties/FlecsReplicationComponentDefinition.h"
#include "Types/SolidNotNull.h"

class UFlecsWorld;

namespace UE::Flecs
{
	using FFlecsReplicationComponentRegistrationFunction = bool(*)(
		const TSolidNotNull<const UFlecsWorld*> InWorld,
		const FFlecsReplicationComponentDefinition& InDefinition,
		FString* OutError);

	using FFlecsReplicationComponentMarkerFunction = void(*)(const FFlecsComponentHandle& InComponent);

	/** Optional core hook installed by UnrealFlecsNetworking. */
	struct UNREALFLECS_API FFlecsComponentRegistrationHooks
	{
		static void InstallReplicationHooks(
			const void* InOwner,
			FFlecsReplicationComponentRegistrationFunction InRegister,
			FFlecsReplicationComponentMarkerFunction InMark);

		static void UninstallReplicationHooks(const void* InOwner);

		static bool RegisterReplicatedComponent(
			const TSolidNotNull<const UFlecsWorld*> InWorld,
			const FFlecsReplicationComponentDefinition& InDefinition,
			FString* OutError = nullptr);

		static void MarkReplicatedComponent(const FFlecsComponentHandle& InComponent);
	}; // struct FFlecsComponentRegistrationHooks
	
} // namespace UE::Flecs
