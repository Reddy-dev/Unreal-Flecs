// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/FlecsReplicationTransportBase.h"

#include "Networking/FlecsNetworkWorldSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsReplicationTransportBase)

bool FFlecsDefaultReplicationInterestPolicy::IsInterested(const FFlecsReplicationRouteDescriptor& Route,
	const FFlecsReplicationConnectionInterestContext& Connection, const FFlecsReplicationConnectionView&) const
{
	switch (Route.Audience)
	{
	case EFlecsReplicationAudience::Everyone:
		return true;
	case EFlecsReplicationAudience::OwnerOnly:
		return Route.Owner.IsValid() && Route.Owner == Connection.Owner;
	case EFlecsReplicationAudience::Team:
		return Route.Team != INDEX_NONE && Route.Team == Connection.Team;
	case EFlecsReplicationAudience::Zone:
		return !Route.Zone.IsNone() && Connection.Zones.Contains(Route.Zone);
	case EFlecsReplicationAudience::Custom:
	default:
		return false;
	}
}

void UFlecsReplicationTransportBase::MigrateEntity(const FFlecsReplicationRouteDescriptor& OldRoute,
	const FFlecsReplicationRouteDescriptor& NewRoute, const FFlecsReplicationLayoutDefinition& Layout,
	const FFlecsReplicatedEntityUpdate& FullUpdate)
{
	PublishLayout(NewRoute, Layout);
	PublishEntity(NewRoute, FullUpdate);
	RemoveEntity(OldRoute, FullUpdate.NetworkId);
}

namespace
{
	TMap<FName, TWeakObjectPtr<UClass>>& GetProviders()
	{
		static TMap<FName, TWeakObjectPtr<UClass>> Providers;
		return Providers;
	}
	
} // namespace

bool UFlecsReplicationTransportBase::InitializeTransport(UFlecsNetworkWorldSubsystem* InSubsystem)
{
	NetworkSubsystem = InSubsystem;
	return IsValid(InSubsystem);
}

void UFlecsReplicationTransportBase::ShutdownTransport()
{
	NetworkSubsystem.Reset();
}

void UFlecsReplicationTransportBase::HandleProtocolError(const FString& Diagnostic)
{
	UE_LOG(LogFlecsCore, Error, TEXT("Flecs replication protocol error: %s"), *Diagnostic);
}

bool FFlecsReplicationTransportRegistry::RegisterProvider(const FName ProviderName, UClass* TransportClass)
{
	if (ProviderName.IsNone() || !TransportClass || !TransportClass->IsChildOf(UFlecsReplicationTransportBase::StaticClass()))
	{
		return false;
	}
	
	TMap<FName, TWeakObjectPtr<UClass>>& Providers = GetProviders();
	
	if (Providers.Contains(ProviderName))
	{
		return false;
	}
	
	Providers.Add(ProviderName, TransportClass);
	return true;
}

void FFlecsReplicationTransportRegistry::UnregisterProvider(const FName ProviderName)
{
	GetProviders().Remove(ProviderName);
}

UClass* FFlecsReplicationTransportRegistry::FindProvider(const FName ProviderName)
{
	const TWeakObjectPtr<UClass>* Found = GetProviders().Find(ProviderName);
	return Found ? Found->Get() : nullptr;
}
