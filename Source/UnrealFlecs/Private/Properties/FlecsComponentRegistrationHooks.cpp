// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Properties/FlecsComponentRegistrationHooks.h"

#include "SolidMacros/Macros.h"

namespace
{
	struct FReplicationHooks
	{
		const void* Owner = nullptr;
		UE::Flecs::FFlecsReplicationComponentRegistrationFunction Register = nullptr;
		UE::Flecs::FFlecsReplicationComponentMarkerFunction Mark = nullptr;
	}; // struct FReplicationHooks

	FReplicationHooks& GetReplicationHooks()
	{
		static FReplicationHooks Hooks;
		return Hooks;
	}
	
} // namespace

void UE::Flecs::FFlecsComponentRegistrationHooks::InstallReplicationHooks(
	const void* InOwner,
	const FFlecsReplicationComponentRegistrationFunction InRegister,
	const FFlecsReplicationComponentMarkerFunction InMark)
{
	solid_cassume(InOwner);
	
	solid_cassume(InRegister);
	solid_cassume(InMark);

	FReplicationHooks& Hooks = GetReplicationHooks();
	solid_cassumef(!Hooks.Owner || Hooks.Owner == InOwner,
		TEXT("Replication component hooks are already installed by another module"));

	Hooks.Owner = InOwner;
	Hooks.Register = InRegister;
	Hooks.Mark = InMark;
}

void UE::Flecs::FFlecsComponentRegistrationHooks::UninstallReplicationHooks(const void* InOwner)
{
	if (!InOwner)
	{
		return;
	}

	FReplicationHooks& Hooks = GetReplicationHooks();
	if (Hooks.Owner != InOwner)
	{
		return;
	}

	Hooks = {};
}

TValueOrError<void, FString> UE::Flecs::FFlecsComponentRegistrationHooks::RegisterReplicatedComponent(
	const TSolidNotNull<const UFlecsWorld*> InWorld,
	const FFlecsReplicationComponentDefinition& InDefinition)
{
	const FReplicationHooks& Hooks = GetReplicationHooks();
	
	if (!Hooks.Register)
	{
		return MakeError(TEXT("No Networking Module loaded"));
	}

	return Hooks.Register(InWorld, InDefinition);
}

void UE::Flecs::FFlecsComponentRegistrationHooks::MarkReplicatedComponent(
	const FFlecsComponentHandle& InComponent)
{
	const FReplicationHooks& Hooks = GetReplicationHooks();
	if LIKELY_IF(Hooks.Mark)
	{
		Hooks.Mark(InComponent);
	}
}
