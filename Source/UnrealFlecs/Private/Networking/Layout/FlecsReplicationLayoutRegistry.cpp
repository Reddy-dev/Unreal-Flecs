// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Layout/FlecsReplicationLayoutRegistry.h"

#include "Networking/FlecsReplicationKey.h"

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

const FFlecsReplicationLayoutDefinition* FFlecsReplicationLayoutRegistry::BuildForEntity(const TSolidNotNull<const UFlecsWorld*> World,
																						 const FFlecsEntityHandle& Entity, 
                                                                                         bool& bOutWasCreated, 
                                                                                         FString& OutError)
{
	bOutWasCreated = false;
	if UNLIKELY_IF(!Entity.IsValid())
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
		FFlecsReplicationKey Key;

		if (!Id.IsPair())
		{
			if (!FFlecsComponentReplicationRegistry::IsEntityReplicationEligible(World, Id))
			{
				continue;
			}
			
			TValueOrError<FFlecsReplicationIndividualKey, FString> ValueOrError =
				FFlecsReplicationIndividualKey::BuildIndividualKey(World, Id);

			if UNLIKELY_IF(ValueOrError.HasError())
			{
				OutError = ValueOrError.GetError();
				return nullptr;
			}
			
			Key.Primary = ValueOrError.GetValue();

			const FFlecsComponentReplicationDescriptor* Descriptor = Registry.Find(Id);
			
			Key.Kind = EFlecsReplicationKeyKind::Component;
			Key.StorageKind = Descriptor && !Descriptor->bIsTag ? EFlecsReplicationKeyStorageKind::Primary 
				: EFlecsReplicationKeyStorageKind::None;
			
			Keys.Add(MoveTemp(Key));
			continue;
		}

		// Pair storage omits entity generations from both elements. Restore the
		// current alive IDs before using them as local registry keys.
		const FFlecsEntityHandle RelationshipEntity = World->GetAlive(Id.GetFirst());
		const FFlecsEntityHandle TargetEntity = World->GetAlive(Id.GetSecond());
		
		const FFlecsId First = RelationshipEntity.IsValid() ? RelationshipEntity.GetFlecsId() : Id.GetFirst();
		const FFlecsId Second = TargetEntity.IsValid() ? TargetEntity.GetFlecsId() : Id.GetSecond();
		
		const EFlecsReplicationKeyStorageKind StorageKind = FFlecsReplicationKey::GetStorageKindForPair(World, First, Second);

		if (StorageKind == EFlecsReplicationKeyStorageKind::None
			&& !FFlecsComponentReplicationRegistry::IsEntityReplicationEligible(World, First))
		{
			continue;
		}

		TValueOrError<FFlecsReplicationIndividualKey, FString> FirstValueOrError =
			FFlecsReplicationIndividualKey::BuildIndividualKey(World, First);
		
		TValueOrError<FFlecsReplicationIndividualKey, FString> SecondValueOrError =
			FFlecsReplicationIndividualKey::BuildIndividualKey(World, Second);

		if UNLIKELY_IF(FirstValueOrError.HasError() || SecondValueOrError.HasError())
		{
			OutError = FirstValueOrError.HasError() ? FirstValueOrError.GetError() : SecondValueOrError.GetError();
			return nullptr;
		}

		Key.Kind = EFlecsReplicationKeyKind::Pair;
		Key.StorageKind = StorageKind;
		Key.Primary = FirstValueOrError.GetValue();
		Key.Secondary = SecondValueOrError.GetValue();
		Keys.Add(MoveTemp(Key));
	}

	Keys.Sort([](const FFlecsReplicationKey& A, const FFlecsReplicationKey& B)
	{
		return A.CanonicalString() < B.CanonicalString();
	});

	FFlecsReplicationLayoutDefinition Definition;
	Definition.Keys = MoveTemp(Keys);
	Definition.LayoutId = ComputeLayoutId(Definition.Keys);
	
	if (const FFlecsReplicationLayoutDefinition* Existing = Definitions.Find(Definition.LayoutId))
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

TValueOrError<void, FString> FFlecsReplicationLayoutRegistry::AddRemoteDefinition(const FFlecsReplicationLayoutDefinition& Definition)
{
	if (!Definition.LayoutId.IsValid() || ComputeLayoutId(Definition.Keys) != Definition.LayoutId)
	{
		return MakeError("Received replication layout has an invalid identity");
	}
	
	if (const FFlecsReplicationLayoutDefinition* Existing = Definitions.Find(Definition.LayoutId))
	{
		if (Existing->Keys != Definition.Keys)
		{
			return MakeError("Received replication layout collides with an existing definition");
		}
		
		return MakeValue();
	}
	
	Definitions.Add(Definition.LayoutId, Definition);
	return MakeValue();
}