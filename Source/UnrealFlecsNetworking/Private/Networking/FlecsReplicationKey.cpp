// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/FlecsReplicationKey.h"

#include "Networking/Subsystem/FlecsNetworkSubsystemSingleton.h"
#include "Networking/Subsystem/FlecsNetworkWorldSubsystem.h"
#include "Entities/FlecsStablePathTag.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsReplicationKey)

FString FFlecsReplicationIndividualKey::CanonicalString() const
{
	FString Result;
	
	switch (Kind)
	{
		case EFlecsReplicationPairTargetKind::None:
			break;
		case EFlecsReplicationPairTargetKind::Schema:
			Result = Schema.ToString();
			break;
		case EFlecsReplicationPairTargetKind::StableSymbolValue:
		case EFlecsReplicationPairTargetKind::StablePathValue:
			Result = StableIdentifier;
			break;
		case EFlecsReplicationPairTargetKind::Entity:
			Result = Entity.GetValue() != 0 ? FString::Printf(TEXT("%llu"), Entity.GetValue()) : FString();
			break;
	}
	
	return Result;
}

const FFlecsComponentReplicationDescriptor* FFlecsReplicationIndividualKey::TryGetDescriptor(
	const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld) const
{
	if (Kind == EFlecsReplicationPairTargetKind::Schema)
	{
		return FFlecsComponentReplicationRegistry::Get(InWorld->GetFlecsWorld()).Find(Schema);
	}
	
	return nullptr;
}

TValueOrError<FFlecsReplicationIndividualKey, FString> FFlecsReplicationIndividualKey::BuildIndividualKey(
	const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld, const FFlecsId InId)
{
	const FFlecsComponentReplicationRegistry& Registry = FFlecsComponentReplicationRegistry::Get(InWorld->GetFlecsWorld());
	
	FFlecsReplicationIndividualKey Result;
	
	if (const FFlecsComponentReplicationDescriptor* Descriptor = Registry.Find(InId))
	{
		Result.Kind = EFlecsReplicationPairTargetKind::Schema;
		Result.Schema = Descriptor->SchemaId;
	}
	
	if UNLIKELY_IF(!ensureAlwaysMsgf(InId.IsValid(), TEXT("Invalid Flecs ID")))
	{
		return MakeError("Invalid Flecs ID");
	}
	
	const FFlecsEntityHandle IdEntityHandle = InWorld->GetAlive(InId);
	if (!ensureAlwaysMsgf(IdEntityHandle.IsValid(), TEXT("Flecs ID does not correspond to a valid entity")))
	{
		return MakeError("Flecs ID does not correspond to a valid entity");
	}
	
	if (const FFlecsNetworkId* NetworkIdComponent = IdEntityHandle.TryGet<FFlecsNetworkId>())
	{
		Result.Kind = EFlecsReplicationPairTargetKind::Entity;
		Result.Entity = *NetworkIdComponent;
	}
	else if (IdEntityHandle.HasSymbol())
	{
		Result.Kind = EFlecsReplicationPairTargetKind::StableSymbolValue;
		Result.StableIdentifier = IdEntityHandle.GetSymbol();
	}
	else if (IdEntityHandle.Has<FFlecsStablePathTag>())
	{
		Result.Kind = EFlecsReplicationPairTargetKind::StablePathValue;
		Result.StableIdentifier = IdEntityHandle.GetPath();
	}

	return MakeValue(Result);
}

FFlecsId FFlecsReplicationIndividualKey::ResolveToId(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld,
	const FFlecsReplicationIndividualKey& InKey)
{
	switch (InKey.Kind)
	{
		case EFlecsReplicationPairTargetKind::Schema:
		{
			if (const FFlecsComponentReplicationDescriptor* Descriptor = InKey.TryGetDescriptor(InWorld))
			{
				return Descriptor->LocalFlecsId;
			}
			break;
		}
		case EFlecsReplicationPairTargetKind::StableSymbolValue:
			{
				const FFlecsEntityHandle EntityHandle = InWorld->LookupEntityBySymbol_Internal(InKey.StableIdentifier);
				if (EntityHandle.IsValid())
				{
					return EntityHandle;
				}
				
				break;
			}
		case EFlecsReplicationPairTargetKind::StablePathValue:
		{
			const FFlecsEntityHandle EntityHandle = InWorld->LookupEntity(InKey.StableIdentifier);
			if (EntityHandle.IsValid())
			{
				return EntityHandle;
			}
			break;
		}
		case EFlecsReplicationPairTargetKind::Entity:
		{
			const TSolidNotNull<UFlecsNetworkWorldSubsystem*> NetworkSubsystem 
					= InWorld->Get<FFlecsNetworkSubsystemSingleton>().GetSubsystemChecked<UFlecsNetworkWorldSubsystem>();
				
			const TOptional<FFlecsEntityHandle> EntityHandle = NetworkSubsystem->GetEntityFromNetworkId(InKey.Entity);
			if LIKELY_IF(IsValid(EntityHandle))
			{
				return EntityHandle.GetValue();
			}
			break;
		}
		default:
			break;
	}

	return FFlecsId();
}

EFlecsReplicationKeyStorageKind FFlecsReplicationKey::GetStorageKindForPair(
	const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld, const FFlecsId InFirstId, const FFlecsId InSecondId)
{
	const FFlecsComponentReplicationDescriptor* FirstDescriptor = FFlecsComponentReplicationRegistry::Get(InWorld->GetFlecsWorld())
		.Find(InFirstId);
	
	if (FirstDescriptor && FirstDescriptor->IsStorageEligible())
	{
		return EFlecsReplicationKeyStorageKind::Primary;
	}
	
	const FFlecsComponentReplicationDescriptor* SecondDescriptor = FFlecsComponentReplicationRegistry::Get(InWorld->GetFlecsWorld())
		.Find(InSecondId);
	
	if (SecondDescriptor && SecondDescriptor->IsStorageEligible())
	{
		return EFlecsReplicationKeyStorageKind::Secondary;
	}
	
	return EFlecsReplicationKeyStorageKind::None;
}

FFlecsId FFlecsReplicationKey::ResolveToId(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld,
	const FFlecsReplicationKey& InKey)
{
	if (InKey.Kind == EFlecsReplicationKeyKind::Component)
	{
		return FFlecsReplicationIndividualKey::ResolveToId(InWorld, InKey.Primary);
	}
	else if (InKey.Kind == EFlecsReplicationKeyKind::Pair)
	{
		const FFlecsId FirstId = FFlecsReplicationIndividualKey::ResolveToId(InWorld, InKey.Primary);
		const FFlecsId SecondId = FFlecsReplicationIndividualKey::ResolveToId(InWorld, InKey.Secondary);
		
		return FFlecsId::MakePair(FirstId, SecondId);
	}
	
	UNREACHABLE
	return FFlecsId();
}

TValueOrError<FFlecsReplicationKey, FString> FFlecsReplicationKey::BuildKey(
	const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld, const FFlecsId InId)
{
	solid_check(InId.IsValid());
	
	if (InId.IsPair())
	{
		const FFlecsId FirstId = InId.GetFirst();
		const FFlecsId SecondId = InId.GetSecond();
		
		const EFlecsReplicationKeyStorageKind StorageKind = GetStorageKindForPair(InWorld, FirstId, SecondId);
		
		if UNLIKELY_IF(StorageKind == EFlecsReplicationKeyStorageKind::None)
		{
			return MakeError("Neither component in the pair is eligible for storage");
		}
		
		const TValueOrError<FFlecsReplicationIndividualKey, FString> FirstKeyResult = FFlecsReplicationIndividualKey::BuildIndividualKey(InWorld, FirstId);
		if UNLIKELY_IF(!FirstKeyResult.IsValid())
		{
			return MakeError(FString::Printf(TEXT("Failed to build first individual key: %s"), *FirstKeyResult.GetError()));
		}
		
		const TValueOrError<FFlecsReplicationIndividualKey, FString> SecondKeyResult = FFlecsReplicationIndividualKey::BuildIndividualKey(InWorld, SecondId);
		if UNLIKELY_IF(!SecondKeyResult.IsValid())
		{
			return MakeError(FString::Printf(TEXT("Failed to build second individual key: %s"), *SecondKeyResult.GetError()));
		}
		
		FFlecsReplicationKey Result;
		Result.Kind = EFlecsReplicationKeyKind::Pair;
		Result.Primary = FirstKeyResult.GetValue();
		Result.Secondary = SecondKeyResult.GetValue();
		Result.StorageKind = StorageKind;
		
		return MakeValue(Result);
	}
	else
	{
		const TValueOrError<FFlecsReplicationIndividualKey, FString> IndividualKeyResult = FFlecsReplicationIndividualKey::BuildIndividualKey(InWorld, InId);
		
		if UNLIKELY_IF(!IndividualKeyResult.IsValid())
		{
			return MakeError(FString::Printf(TEXT("Failed to build individual key: %s"), *IndividualKeyResult.GetError()));
		}
		
		FFlecsReplicationKey Result;
		Result.Kind = EFlecsReplicationKeyKind::Component;
		Result.Primary = IndividualKeyResult.GetValue();

		const FFlecsComponentReplicationDescriptor* Descriptor =
			FFlecsComponentReplicationRegistry::Get(InWorld->GetFlecsWorld()).Find(InId);
		Result.StorageKind = Descriptor && Descriptor->IsStorageEligible()
			? EFlecsReplicationKeyStorageKind::Primary
			: EFlecsReplicationKeyStorageKind::None;

		return MakeValue(Result);
	}
}

FString FFlecsReplicationKey::CanonicalString() const
{
	/*return FString::Printf(TEXT("%u|%s|%u|%s|%u|%u|%s|%u|%s|%llu|%u"),
		static_cast<uint8>(Kind), *RelationshipSchema.ToString(), RelationshipVersion,
		*StorageSchema.ToString(), StorageVersion, static_cast<uint8>(TargetKind),
		*TargetSchema.ToString(), TargetVersion, *StableTargetIdentifier,
		EntityTarget.GetValue(), bHasPayload ? 1u : 0u);*/
	
	FString Result;
	
	if (Kind == EFlecsReplicationKeyKind::Component)
	{
		Result = Primary.CanonicalString();
	}
	else
	{
		Result = FString::Printf(TEXT("%s|%s"), *Primary.CanonicalString(), *Secondary.CanonicalString());
	}
	
	Result += FString::Printf(TEXT("|%u"), static_cast<uint8>(StorageKind));
	
	return Result;
}

const FFlecsComponentReplicationDescriptor* FFlecsReplicationKey::TryGetStorageDescriptor(
	const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld) const
{
	// @TODO: Convert to switch
	if (StorageKind == EFlecsReplicationKeyStorageKind::Primary)
	{
		return Primary.TryGetDescriptor(InWorld);
	}
	else if (StorageKind == EFlecsReplicationKeyStorageKind::Secondary)
	{
		return Secondary.TryGetDescriptor(InWorld);
	}
	
	return nullptr;
}
