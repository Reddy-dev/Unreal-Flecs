// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/FlecsReplicationTransportBase.h"

#include "Networking/FlecsNetworkWorldSubsystem.h"
#include "UObject/PropertyOptional.h"
#include "UObject/UnrealType.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsReplicationTransportBase)

namespace
{
	TMap<FName, TUniquePtr<IFlecsReplicationInterestPolicy>>& GetInterestPolicies()
	{
		static TMap<FName, TUniquePtr<IFlecsReplicationInterestPolicy>> Policies;
		return Policies;
	}

	bool ContainsUnstableObjectReference(const FProperty* Property, TSet<const UStruct*>& VisitedStructs);

	bool StructContainsUnstableObjectReference(const UStruct* Struct, TSet<const UStruct*>& VisitedStructs)
	{
		if (!Struct || VisitedStructs.Contains(Struct))
		{
			return false;
		}

		VisitedStructs.Add(Struct);
		for (TFieldIterator<FProperty> Iterator(Struct); Iterator; ++Iterator)
		{
			if (ContainsUnstableObjectReference(*Iterator, VisitedStructs))
			{
				return true;
			}
		}
		return false;
	}

	bool ContainsUnstableObjectReference(const FProperty* Property, TSet<const UStruct*>& VisitedStructs)
	{
		if (CastField<FObjectPropertyBase>(Property)
			|| CastField<FInterfaceProperty>(Property)
			|| CastField<FDelegateProperty>(Property)
			|| CastField<FMulticastDelegateProperty>(Property))
		{
			return true;
		}
		if (const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
		{
			return ContainsUnstableObjectReference(ArrayProperty->Inner, VisitedStructs);
		}
		if (const FSetProperty* SetProperty = CastField<FSetProperty>(Property))
		{
			return ContainsUnstableObjectReference(SetProperty->ElementProp, VisitedStructs);
		}
		if (const FMapProperty* MapProperty = CastField<FMapProperty>(Property))
		{
			return ContainsUnstableObjectReference(MapProperty->KeyProp, VisitedStructs)
				|| ContainsUnstableObjectReference(MapProperty->ValueProp, VisitedStructs);
		}
		if (const FOptionalProperty* OptionalProperty = CastField<FOptionalProperty>(Property))
		{
			return ContainsUnstableObjectReference(OptionalProperty->GetValueProperty(), VisitedStructs);
		}
		if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
		{
			return StructContainsUnstableObjectReference(StructProperty->Struct, VisitedStructs);
		}
		return false;
	}

} // namespace

FFlecsEveryoneReplicationInterestPolicy::FFlecsEveryoneReplicationInterestPolicy()
	: TFlecsReplicationInterestPolicy<FFlecsReplicationEveryoneInterestDescriptor>(
		FFlecsReplicationInterestPolicyNames::Everyone())
{
}

bool FFlecsEveryoneReplicationInterestPolicy::IsInterested(
	const FFlecsReplicationEveryoneInterestDescriptor&,
	const FFlecsReplicationInterestEvaluationQuery&) const
{
	return true;
}

FFlecsOwnerReplicationInterestPolicy::FFlecsOwnerReplicationInterestPolicy()
	: TFlecsReplicationInterestPolicy<FFlecsReplicationOwnerInterestDescriptor>(
		FFlecsReplicationInterestPolicyNames::Owner())
{
}

bool FFlecsOwnerReplicationInterestPolicy::ValidateDescriptor(
	const FFlecsReplicationOwnerInterestDescriptor& Descriptor, FString& OutError) const
{
	if (!Descriptor.Owner.IsValid())
	{
		OutError = TEXT("Owner interest requires a valid owner network ID");
		return false;
	}
	return true;
}

bool FFlecsOwnerReplicationInterestPolicy::IsInterested(
	const FFlecsReplicationOwnerInterestDescriptor& Descriptor,
	const FFlecsReplicationInterestEvaluationQuery& Query) const
{
	const FFlecsReplicationOwnerInterestFragment* Fragment =
		Query.Context.Find<FFlecsReplicationOwnerInterestFragment>();
	return Fragment && Fragment->Owner == Descriptor.Owner;
}

FFlecsTeamReplicationInterestPolicy::FFlecsTeamReplicationInterestPolicy()
	: TFlecsReplicationInterestPolicy<FFlecsReplicationTeamInterestDescriptor>(
		FFlecsReplicationInterestPolicyNames::Team())
{
}

bool FFlecsTeamReplicationInterestPolicy::ValidateDescriptor(
	const FFlecsReplicationTeamInterestDescriptor& Descriptor, FString& OutError) const
{
	if (Descriptor.Team == INDEX_NONE)
	{
		OutError = TEXT("Team interest requires a valid team value");
		return false;
	}
	return true;
}

bool FFlecsTeamReplicationInterestPolicy::IsInterested(
	const FFlecsReplicationTeamInterestDescriptor& Descriptor,
	const FFlecsReplicationInterestEvaluationQuery& Query) const
{
	const FFlecsReplicationTeamInterestFragment* Fragment =
		Query.Context.Find<FFlecsReplicationTeamInterestFragment>();
	return Fragment && Fragment->Team == Descriptor.Team;
}

FFlecsZoneReplicationInterestPolicy::FFlecsZoneReplicationInterestPolicy()
	: TFlecsReplicationInterestPolicy<FFlecsReplicationZoneInterestDescriptor>(
		FFlecsReplicationInterestPolicyNames::Zone())
{
}

bool FFlecsZoneReplicationInterestPolicy::ValidateDescriptor(
	const FFlecsReplicationZoneInterestDescriptor& Descriptor, FString& OutError) const
{
	if (Descriptor.Zone.IsNone())
	{
		OutError = TEXT("Zone interest requires a non-empty zone name");
		return false;
	}
	return true;
}

bool FFlecsZoneReplicationInterestPolicy::IsInterested(
	const FFlecsReplicationZoneInterestDescriptor& Descriptor,
	const FFlecsReplicationInterestEvaluationQuery& Query) const
{
	const FFlecsReplicationZoneInterestFragment* Fragment =
		Query.Context.Find<FFlecsReplicationZoneInterestFragment>();
	return Fragment && Fragment->Zones.Contains(Descriptor.Zone);
}

bool FFlecsReplicationInterestPolicyRegistry::RegisterPolicy(
	TUniquePtr<IFlecsReplicationInterestPolicy> Policy)
{
	if (!Policy || Policy->GetPolicyName().IsNone() || !Policy->GetDescriptorStruct()
		|| Policy->GetDescriptorStruct() == FFlecsReplicationInterestDescriptorBase::StaticStruct()
		|| !Policy->GetDescriptorStruct()->IsChildOf(FFlecsReplicationInterestDescriptorBase::StaticStruct()))
	{
		return false;
	}

	TMap<FName, TUniquePtr<IFlecsReplicationInterestPolicy>>& Policies = GetInterestPolicies();
	const FName PolicyName = Policy->GetPolicyName();
	if (Policies.Contains(PolicyName))
	{
		return false;
	}

	Policies.Add(PolicyName, MoveTemp(Policy));
	return true;
}

bool FFlecsReplicationInterestPolicyRegistry::UnregisterPolicy(const FName PolicyName)
{
	return GetInterestPolicies().Remove(PolicyName) > 0;
}

const IFlecsReplicationInterestPolicy* FFlecsReplicationInterestPolicyRegistry::FindPolicy(
	const FName PolicyName)
{
	const TUniquePtr<IFlecsReplicationInterestPolicy>* Policy = GetInterestPolicies().Find(PolicyName);
	return Policy ? Policy->Get() : nullptr;
}

bool FFlecsReplicationInterestPolicyRegistry::ValidateBinding(
	const FFlecsReplicationInterestBinding& Binding, FString& OutError)
{
	OutError.Reset();
	if (Binding.PolicyName.IsNone())
	{
		OutError = TEXT("Interest binding has no policy name");
		return false;
	}

	const IFlecsReplicationInterestPolicy* Policy = FindPolicy(Binding.PolicyName);
	if (!Policy)
	{
		OutError = FString::Printf(TEXT("Interest policy '%s' is not registered"),
			*Binding.PolicyName.ToString());
		return false;
	}

	const UScriptStruct* DescriptorStruct = Binding.Descriptor.GetScriptStruct();
	if (!Binding.Descriptor.IsValid() || !DescriptorStruct
		|| !DescriptorStruct->IsChildOf(FFlecsReplicationInterestDescriptorBase::StaticStruct()))
	{
		OutError = FString::Printf(TEXT("Interest policy '%s' has an invalid descriptor"),
			*Binding.PolicyName.ToString());
		return false;
	}
	if (DescriptorStruct != Policy->GetDescriptorStruct())
	{
		OutError = FString::Printf(TEXT("Interest policy '%s' expects descriptor '%s', received '%s'"),
			*Binding.PolicyName.ToString(), *Policy->GetDescriptorStruct()->GetPathName(),
			*DescriptorStruct->GetPathName());
		return false;
	}

	TSet<const UStruct*> VisitedStructs;
	if (StructContainsUnstableObjectReference(DescriptorStruct, VisitedStructs))
	{
		OutError = FString::Printf(TEXT("Interest descriptor '%s' contains an unstable object reference"),
			*DescriptorStruct->GetPathName());
		return false;
	}

	if (!Policy->ValidateDescriptor(Binding.Descriptor, OutError))
	{
		if (OutError.IsEmpty())
		{
			OutError = FString::Printf(TEXT("Interest policy '%s' rejected its descriptor"),
				*Binding.PolicyName.ToString());
		}
		return false;
	}
	return true;
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
