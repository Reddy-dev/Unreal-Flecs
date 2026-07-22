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

TValueOrError<const FFlecsReplicationLayoutDefinition*, FString> FFlecsReplicationLayoutRegistry::BuildForEntity(
																		const TSolidNotNull<const UFlecsWorldInterfaceObject*> World,
																		const FFlecsEntityHandle& Entity)
{
	if UNLIKELY_IF(!ensureAlways(Entity.IsValid()))
	{
		return MakeError(TEXT("Cannot build a replication layout for an invalid world/entity"));
	}

	const flecs::table_t* Table = Entity.GetEntity().table().get_table();
	
	if (const FFlecsReplicationLayoutId* CachedId = TableCache.Find(Table))
	{
		if (const FFlecsReplicationLayoutDefinition* Definition = Definitions.Find(*CachedId))
		{
			return MakeValue(Definition);
		}
		else UNLIKELY_ATTRIBUTE
		{
			return MakeError(FString::Printf(TEXT("Replication layout definition for cached layout ID %s not found"),
				*CachedId->ToString()));
		}
	}

	const FFlecsComponentReplicationRegistry& Registry = FFlecsComponentReplicationRegistry::Get(World->GetFlecsWorld());
	TArray<FFlecsReplicationKey> Keys;
	
	for (const FFlecsId Id : Entity.GetType())
	{
		FFlecsReplicationKey Key;

		if (!Id.IsPair())
		{
			if (!FFlecsComponentReplicationRegistry::IsEntityReplicationEligible(World->GetFlecsWorld(), Id))
			{
				continue;
			}
			
			TValueOrError<FFlecsReplicationIndividualKey, FString> ValueOrError =
				FFlecsReplicationIndividualKey::BuildIndividualKey(World, Id);

			if UNLIKELY_IF(ValueOrError.HasError())
			{
				return MakeError(ValueOrError.GetError());
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
			&& !FFlecsComponentReplicationRegistry::IsEntityReplicationEligible(World->GetFlecsWorld(), First))
		{
			continue;
		}

		TValueOrError<FFlecsReplicationIndividualKey, FString> FirstValueOrError =
			FFlecsReplicationIndividualKey::BuildIndividualKey(World, First);
		
		TValueOrError<FFlecsReplicationIndividualKey, FString> SecondValueOrError =
			FFlecsReplicationIndividualKey::BuildIndividualKey(World, Second);

		if UNLIKELY_IF(FirstValueOrError.HasError() || SecondValueOrError.HasError())
		{
			return MakeError(FirstValueOrError.HasError() ? FirstValueOrError.GetError() : SecondValueOrError.GetError());
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
			return MakeError(FString::Printf(TEXT("Replication layout hash collision for %s"), *Definition.LayoutId.ToString()));
		}
		
		TableCache.Add(Table, Definition.LayoutId);
		return MakeValue(Existing);
	}

	const FFlecsReplicationLayoutId Id = Definition.LayoutId;
	Definitions.Add(Id, MoveTemp(Definition));
	TableCache.Add(Table, Id);
	
	UE_LOGFMT(LogFlecsCore, Verbose,
		"Built replication layout for entity {Entity} with layout ID {LayoutId} and {KeyCount} keys",
		*Entity.ToString(), *Id.ToString(), Definitions[Id].Keys.Num());
	return MakeValue(Definitions.Find(Id));
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