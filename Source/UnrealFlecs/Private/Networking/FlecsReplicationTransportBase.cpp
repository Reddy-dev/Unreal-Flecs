// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/FlecsReplicationTransportBase.h"

#include "Networking/FlecsNetworkWorldSubsystem.h"
#include "UObject/UnrealType.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsReplicationTransportBase)

namespace
{
	TMap<FName, TUniquePtr<IFlecsReplicationInterestPolicy>>& GetInterestPolicies()
	{
		static TMap<FName, TUniquePtr<IFlecsReplicationInterestPolicy>> Policies;
		return Policies;
	}

	bool PropertyHasForbiddenReference(const FProperty* Property, TSet<const UStruct*>& Visiting);

	bool HasForbiddenReference(const UStruct* Struct, TSet<const UStruct*>& Visiting)
	{
		if (!Struct || Visiting.Contains(Struct))
		{
			return false;
		}
		Visiting.Add(Struct);

		for (TFieldIterator<FProperty> It(Struct); It; ++It)
		{
			if (PropertyHasForbiddenReference(*It, Visiting))
			{
				return true;
			}
		}

		return false;
	}

	bool PropertyHasForbiddenReference(const FProperty* Property, TSet<const UStruct*>& Visiting)
	{
		if (!Property)
		{
			return false;
		}
		if (Property->IsA<FObjectPropertyBase>() || Property->IsA<FInterfaceProperty>()
			|| Property->IsA<FDelegateProperty>() || Property->IsA<FMulticastDelegateProperty>())
		{
			return true;
		}
		if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
		{
			return HasForbiddenReference(StructProperty->Struct, Visiting);
		}
		if (const FArrayProperty* Array = CastField<FArrayProperty>(Property))
		{
			return PropertyHasForbiddenReference(Array->Inner, Visiting);
		}
		if (const FSetProperty* Set = CastField<FSetProperty>(Property))
		{
			return PropertyHasForbiddenReference(Set->ElementProp, Visiting);
		}
		if (const FMapProperty* Map = CastField<FMapProperty>(Property))
		{
			return PropertyHasForbiddenReference(Map->KeyProp, Visiting)
				|| PropertyHasForbiddenReference(Map->ValueProp, Visiting);
		}
		return false;
	}

	TMap<FName, TWeakObjectPtr<UClass>>& GetProviders()
	{
		static TMap<FName, TWeakObjectPtr<UClass>> Providers;
		return Providers;
	}
	
} // namespace

FFlecsEveryoneReplicationInterestPolicy::FFlecsEveryoneReplicationInterestPolicy()
	: TFlecsReplicationInterestPolicy(FFlecsReplicationInterestPolicyNames::Everyone)
{
}

bool FFlecsEveryoneReplicationInterestPolicy::IsInterested(
	const FFlecsReplicationEveryoneInterestDescriptor&, const FFlecsReplicationInterestEvaluationQuery&) const
{
	return true;
}

FFlecsOwnerReplicationInterestPolicy::FFlecsOwnerReplicationInterestPolicy()
	: TFlecsReplicationInterestPolicy(FFlecsReplicationInterestPolicyNames::Owner)
{
}

bool FFlecsOwnerReplicationInterestPolicy::ValidateTypedDescriptor(
	const FFlecsReplicationOwnerInterestDescriptor& Descriptor, FString& OutError) const
{
	if (!Descriptor.OwnerConnection.IsValid())
	{
		OutError = TEXT("Owner interest requires a valid parent connection ID");
		return false;
	}
	return true;
}

bool FFlecsOwnerReplicationInterestPolicy::IsInterested(
	const FFlecsReplicationOwnerInterestDescriptor& Descriptor,
	const FFlecsReplicationInterestEvaluationQuery& Query) const
{
	return Descriptor.OwnerConnection == Query.ConnectionId;
}

FFlecsSpatialCellReplicationInterestPolicy::FFlecsSpatialCellReplicationInterestPolicy()
	: TFlecsReplicationInterestPolicy(FFlecsReplicationInterestPolicyNames::SpatialCell)
{
}

bool FFlecsSpatialCellReplicationInterestPolicy::ValidateTypedDescriptor(
	const FFlecsReplicationSpatialCellInterestDescriptor& Descriptor, FString& OutError) const
{
	if (!FMath::IsFinite(Descriptor.CellSize) || Descriptor.CellSize <= UE_SMALL_NUMBER
		|| !FMath::IsFinite(Descriptor.BubbleRadius) || Descriptor.BubbleRadius < 0.0f)
	{
		OutError = TEXT("SpatialCell requires a finite positive cell size and non-negative bubble radius");
		return false;
	}
	return true;
}

bool FFlecsSpatialCellReplicationInterestPolicy::IsInterested(
	const FFlecsReplicationSpatialCellInterestDescriptor& Descriptor,
	const FFlecsReplicationInterestEvaluationQuery& Query) const
{
	const FVector CellMin = FVector(Descriptor.Cell) * Descriptor.CellSize;
	const FVector CellMax = CellMin + FVector(Descriptor.CellSize);
	const FBox ExpandedCell(CellMin - FVector(Descriptor.BubbleRadius),
		CellMax + FVector(Descriptor.BubbleRadius));

	for (const FVector& Position : Query.View.Positions)
	{
		if (ExpandedCell.IsInsideOrOn(Position))
		{
			return true;
		}
	}
	return false;
}

bool FFlecsReplicationInterestPolicyRegistry::RegisterPolicy(
	TUniquePtr<IFlecsReplicationInterestPolicy> Policy)
{
	if (!Policy || Policy->GetPolicyName().IsNone() || !Policy->GetDescriptorStruct()
		|| GetInterestPolicies().Contains(Policy->GetPolicyName()))
	{
		return false;
	}
	GetInterestPolicies().Add(Policy->GetPolicyName(), MoveTemp(Policy));
	return true;
}

bool FFlecsReplicationInterestPolicyRegistry::UnregisterPolicy(const FName PolicyName)
{
	return GetInterestPolicies().Remove(PolicyName) > 0;
}

const IFlecsReplicationInterestPolicy* FFlecsReplicationInterestPolicyRegistry::FindPolicy(
	const FName PolicyName)
{
	const TUniquePtr<IFlecsReplicationInterestPolicy>* Found = GetInterestPolicies().Find(PolicyName);
	return Found ? Found->Get() : nullptr;
}

bool FFlecsReplicationInterestPolicyRegistry::ValidateBinding(
	const FFlecsReplicationInterestBinding& Binding, FString& OutError)
{
	const IFlecsReplicationInterestPolicy* Policy = FindPolicy(Binding.PolicyName);
	if (!Policy)
	{
		OutError = FString::Printf(TEXT("Interest policy '%s' is not registered"), *Binding.PolicyName.ToString());
		return false;
	}

	TSet<const UStruct*> Visiting;
	if (HasForbiddenReference(Binding.Descriptor.GetScriptStruct(), Visiting))
	{
		OutError = FString::Printf(TEXT("Interest descriptor '%s' contains an object, interface, or delegate reference"),
			*Binding.Descriptor.GetScriptStruct()->GetPathName());
		return false;
	}

	return Policy->ValidateDescriptor(Binding.Descriptor, OutError);
}

void UFlecsReplicationTransportBase::MigrateEntity(const FFlecsReplicationRouteDescriptor& OldRoute,
	const FFlecsReplicationRouteDescriptor& NewRoute, const FFlecsReplicationLayoutDefinition& Layout,
	const FFlecsReplicatedEntityUpdate& FullUpdate)
{
	PublishLayout(NewRoute, Layout);
	PublishEntity(NewRoute, FullUpdate);
	RemoveEntity(OldRoute, FullUpdate.NetworkId);
}

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
