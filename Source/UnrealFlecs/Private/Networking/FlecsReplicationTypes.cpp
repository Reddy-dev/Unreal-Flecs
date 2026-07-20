// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/FlecsReplicationTypes.h"

#include "Misc/SecureHash.h"
#include "Networking/FlecsStablePathTag.h"
#include "Serialization/MemoryWriter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsReplicationTypes)

namespace
{
	NO_DISCARD EFlecsReplicationKeyStorageKind GetStorageKindForPair(const FFlecsComponentReplicationRegistry& Registry,
		const FFlecsId First, const FFlecsId Second)
	{
		const FFlecsComponentReplicationDescriptor* FirstDescriptor = Registry.Find(First);
		if (FirstDescriptor && !FirstDescriptor->bIsTag)
		{
			return EFlecsReplicationKeyStorageKind::Primary;
		}
		
		const FFlecsComponentReplicationDescriptor* SecondDescriptor = Registry.Find(Second);
		if (SecondDescriptor && !SecondDescriptor->bIsTag)
		{
			return EFlecsReplicationKeyStorageKind::Secondary;
		}
		
		return EFlecsReplicationKeyStorageKind::None;
	}
	
} // namespace

FFlecsReplicationLayoutId FFlecsReplicationLayoutRegistry::ComputeLayoutId(
	const TArray<FFlecsReplicationKey>& Keys)
{
	FMD5 Md5;
	
	for (const FFlecsReplicationKey& Key : Keys)
	{
		const FString Canonical = Key.CanonicalString();
		FTCHARToUTF8 Utf8(*Canonical);
		Md5.Update(reinterpret_cast<const uint8*>(Utf8.Get()), Utf8.Length());
		const uint8 Separator = 0;
		Md5.Update(&Separator, 1);
	}
	
	FMD5Hash Hash;
	
	Hash.Set(Md5);
	FGuid Guid = MD5HashToGuid(Hash);
	
	if (!Guid.IsValid())
	{
		Guid.D = 1;
	}
	
	return FFlecsReplicationLayoutId(Guid);
}


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
	const TSolidNotNull<const UFlecsWorld*> InWorld) const
{
	if (Kind == EFlecsReplicationPairTargetKind::Schema)
	{
		return FFlecsComponentReplicationRegistry::Get(InWorld).Find(Schema);
	}
	
	return nullptr;
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
	const TSolidNotNull<const UFlecsWorld*> InWorld) const
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

const FFlecsReplicationLayoutDefinition* FFlecsReplicationLayoutRegistry::BuildForEntity(const TSolidNotNull<const UFlecsWorld*> World,
                                                                                         const FFlecsEntityHandle& Entity, bool& bOutWasCreated, FString& OutError)
{
	bOutWasCreated = false;
	if (!Entity.IsValid())
	{
		OutError = TEXT("Cannot build a replication layout for an invalid world/entity");
		return nullptr;
	}

	const flecs::table_t* Table = Entity.GetEntity().table().get_table();
	
	if (const FFlecsReplicationLayoutId* CachedId = TableCache.Find(Table))
	{
		return Definitions.Find(*CachedId);
	}

	const FFlecsComponentReplicationRegistry& Registry = FFlecsComponentReplicationRegistry::Get(World);
	TArray<FFlecsReplicationKey> Keys;
	
	// @TODO: add DontFragment support.
	/*World->CreateQueryBuilder()
		.With(flecs::DontFragment).Src("$Component")
		.With("$Compoent")*/
	
	for (const FFlecsId Id : Entity.GetType())
	{
		const FFlecsComponentReplicationDescriptor* Descriptor = Registry.Find(Id);
			
		if (!Descriptor)
		{
			continue;
		}
			
		// @TODO: Add Other Primary Support
		FFlecsReplicationKey& Key = Keys.AddDefaulted_GetRef();
			
		Key.Primary.Kind = EFlecsReplicationPairTargetKind::Schema;
		Key.Primary.Schema = Descriptor->SchemaId;
		
		
		if (!Id.IsPair())
		{
			Key.Kind = EFlecsReplicationKeyKind::Component;
			Key.StorageKind = !Descriptor->bIsTag ? EFlecsReplicationKeyStorageKind::Primary : EFlecsReplicationKeyStorageKind::None;
			
			continue;
		}
		
		Key.Kind = EFlecsReplicationKeyKind::Pair;
		
		// Pair storage omits entity generations from both elements. Restore the
		// current alive IDs before using them as local registry keys.
		const FFlecsEntityHandle RelationshipEntity = World->GetAlive(Id.GetFirst());
		const FFlecsEntityHandle Target = World->GetAlive(Id.GetSecond());
		
		// how the fuck did you do this bro
		solid_checkf(RelationshipEntity.IsValid(), 
			TEXT("Cannot build a replication layout for a pair with an unknown relationship: %s"), *Id.ToString());
		solid_checkf(Target.IsValid(),
			TEXT("Cannot build a replication layout for a pair with an unknown target: %s"), *Id.ToString());
		
		const FFlecsId First = RelationshipEntity.IsValid() ? RelationshipEntity.GetFlecsId() : Id.GetFirst();
		const FFlecsId Second = Target.IsValid() ? Target.GetFlecsId() : Id.GetSecond();
		
		const EFlecsReplicationKeyStorageKind StorageKind = GetStorageKindForPair(Registry, First, Second);
		
		Key.StorageKind = StorageKind;

		const FFlecsComponentReplicationDescriptor* TargetDescriptor = Registry.Find(Second);
		if (TargetDescriptor)
		{
			Key.Secondary.Kind = EFlecsReplicationPairTargetKind::Schema;
			Key.Secondary.Schema = TargetDescriptor->SchemaId;
		}
		else
		{
			if UNLIKELY_IF (!Target.IsValid())
			{
				OutError = FString::Printf(TEXT("Cannot build a replication layout for a pair with an unknown target: %s"), 
					*Id.ToString());
				return nullptr;
			}
			
			if (Target.Has<FFlecsNetworkId>())
			{
				Key.Secondary.Kind = EFlecsReplicationPairTargetKind::Entity;
				Key.Secondary.Entity = Target.Get<FFlecsNetworkId>();
			}
			else if (Target.HasSymbol())
			{
				Key.Secondary.Kind = EFlecsReplicationPairTargetKind::StableSymbolValue;
				Key.Secondary.StableIdentifier = Target.GetSymbol();
			}
			else if (Target.Has<FFlecsStablePathTag>() && Target.HasName())
			{
				Key.Secondary.Kind = EFlecsReplicationPairTargetKind::StablePathValue;
				Key.Secondary.StableIdentifier = Target.GetPath();
			}
			else UNLIKELY_ATTRIBUTE
			{
				OutError = FString::Printf(TEXT("Cannot build a replication layout for a pair with an unknown target: %s"), 
					*Id.ToString());
				return nullptr;
			}
		}
	}

	Keys.Sort([](const FFlecsReplicationKey& A, const FFlecsReplicationKey& B)
	{
		return A.CanonicalString() < B.CanonicalString();
	});

	FFlecsReplicationLayoutDefinition Definition;
	Definition.Keys = MoveTemp(Keys);
	Definition.LayoutId = ComputeLayoutId(Definition.Keys);
	
	if (FFlecsReplicationLayoutDefinition* Existing = Definitions.Find(Definition.LayoutId))
	{
		if (Existing->Keys != Definition.Keys)
		{
			OutError = FString::Printf(TEXT("Replication layout hash collision for %s"), *Definition.LayoutId.ToString());
			return nullptr;
		}
		
		TableCache.Add(Table, Definition.LayoutId);
		return Existing;
	}

	const FFlecsReplicationLayoutId Id = Definition.LayoutId;
	Definitions.Add(Id, MoveTemp(Definition));
	TableCache.Add(Table, Id);
	
	bOutWasCreated = true;
	
	return Definitions.Find(Id);
}

const FFlecsReplicationLayoutDefinition* FFlecsReplicationLayoutRegistry::Find(const FFlecsReplicationLayoutId Id) const
{
	return Definitions.Find(Id);
}

bool FFlecsReplicationLayoutRegistry::AddRemoteDefinition(const FFlecsReplicationLayoutDefinition& Definition,
	FString& OutError)
{
	if (!Definition.LayoutId.IsValid() || ComputeLayoutId(Definition.Keys) != Definition.LayoutId)
	{
		OutError = TEXT("Received replication layout has an invalid identity");
		return false;
	}
	
	if (const FFlecsReplicationLayoutDefinition* Existing = Definitions.Find(Definition.LayoutId))
	{
		if (Existing->Keys != Definition.Keys)
		{
			OutError = TEXT("Received replication layout collides with an existing definition");
			return false;
		}
		return true;
	}
	
	Definitions.Add(Definition.LayoutId, Definition);
	return true;
}
