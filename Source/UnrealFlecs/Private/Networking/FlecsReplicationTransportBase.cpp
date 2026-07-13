// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/FlecsReplicationTransportBase.h"

#include "Networking/FlecsNetworkWorldSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsReplicationTransportBase)

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
