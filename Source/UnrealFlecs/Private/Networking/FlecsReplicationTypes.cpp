// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/FlecsReplicationTypes.h"

#include "Misc/SecureHash.h"
#include "Serialization/MemoryWriter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsReplicationTypes)

namespace
{
	const FFlecsComponentReplicationDescriptor* GetPairStorageDescriptor(
		const FFlecsComponentReplicationRegistry& Registry, const FFlecsId First, const FFlecsId Second)
	{
		const FFlecsComponentReplicationDescriptor* FirstDescriptor = Registry.Find(First);
		if (FirstDescriptor && !FirstDescriptor->bIsTag) return FirstDescriptor;
		const FFlecsComponentReplicationDescriptor* SecondDescriptor = Registry.Find(Second);
		if (SecondDescriptor && !SecondDescriptor->bIsTag) return SecondDescriptor;
		return FirstDescriptor ? FirstDescriptor : SecondDescriptor;
	}
}

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
	if (!Guid.IsValid()) Guid.D = 1;
	return FFlecsReplicationLayoutId(Guid);
}

FString FFlecsReplicationKey::CanonicalString() const
{
	return FString::Printf(TEXT("%u|%s|%u|%s|%u|%u|%s|%u|%s|%llu|%u"),
		static_cast<uint8>(Kind), *RelationshipSchema.ToString(), RelationshipVersion,
		*StorageSchema.ToString(), StorageVersion, static_cast<uint8>(TargetKind),
		*TargetSchema.ToString(), TargetVersion, *StableTargetName,
		EntityTarget.GetValue(), bHasPayload ? 1u : 0u);
}

const FFlecsReplicationLayoutDefinition* FFlecsReplicationLayoutRegistry::BuildForEntity(const UFlecsWorld* World,
	const FFlecsEntityHandle& Entity, bool& bOutWasCreated, FString& OutError)
{
	bOutWasCreated = false;
	if (!World || !Entity.IsValid())
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
	for (const FFlecsId Id : Entity.GetType())
	{
		if (!Id.IsPair())
		{
			const FFlecsComponentReplicationDescriptor* Descriptor = Registry.Find(Id);
			if (!Descriptor) continue;
			FFlecsReplicationKey& Key = Keys.AddDefaulted_GetRef();
			Key.Kind = EFlecsReplicationKeyKind::Component;
			Key.StorageSchema = Descriptor->SchemaId;
			Key.StorageVersion = Descriptor->SchemaVersion;
			Key.bHasPayload = !Descriptor->bIsTag;
			continue;
		}

		const FFlecsId First = Id.GetFirst();
		const FFlecsId Second = Id.GetSecond();
		const FFlecsComponentReplicationDescriptor* Relationship = Registry.Find(First);
		const FFlecsComponentReplicationDescriptor* Storage = GetPairStorageDescriptor(Registry, First, Second);
		if (!Relationship || !Storage) continue;

		FFlecsReplicationKey& Key = Keys.AddDefaulted_GetRef();
		Key.Kind = EFlecsReplicationKeyKind::Pair;
		Key.RelationshipSchema = Relationship->SchemaId;
		Key.RelationshipVersion = Relationship->SchemaVersion;
		Key.StorageSchema = Storage->SchemaId;
		Key.StorageVersion = Storage->SchemaVersion;
		Key.bHasPayload = !Storage->bIsTag;

		const FFlecsComponentReplicationDescriptor* TargetDescriptor = Registry.Find(Second);
		if (TargetDescriptor)
		{
			Key.TargetKind = EFlecsReplicationPairTargetKind::Schema;
			Key.TargetSchema = TargetDescriptor->SchemaId;
			Key.TargetVersion = TargetDescriptor->SchemaVersion;
		}
		else
		{
			const FFlecsEntityHandle Target = World->IsAlive(Second) ? World->GetAlive(Second) : FFlecsEntityHandle::Invalid();
			if (Target.IsValid() && Target.Has<FFlecsNetworkId>())
			{
				Key.TargetKind = EFlecsReplicationPairTargetKind::Entity;
				Key.EntityTarget = Target.Get<FFlecsNetworkId>();
			}
			else
			{
				Key.TargetKind = EFlecsReplicationPairTargetKind::StableValue;
				Key.StableTargetName = Target.IsValid() ? Target.GetPath() : FString::Printf(TEXT("#%llu"), Second.GetId());
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
