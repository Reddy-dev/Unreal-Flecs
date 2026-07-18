// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/FlecsReplicationTypes.h"

#include "Misc/SecureHash.h"
#include "Networking/FlecsStablePathTag.h"
#include "Networking/FlecsNetworkingStats.h"
#include "Serialization/MemoryWriter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsReplicationTypes)

FName FFlecsReplicationInterestPolicyNames::Everyone()
{
	static const FName Name(TEXT("Everyone"));
	return Name;
}

FName FFlecsReplicationInterestPolicyNames::Owner()
{
	static const FName Name(TEXT("Owner"));
	return Name;
}

FName FFlecsReplicationInterestPolicyNames::Zone()
{
	static const FName Name(TEXT("Zone"));
	return Name;
}

FFlecsReplicationInterestBinding::FFlecsReplicationInterestBinding()
	: PolicyName(FFlecsReplicationInterestPolicyNames::Everyone())
{
	Descriptor.InitializeAs<FFlecsReplicationEveryoneInterestDescriptor>();
}

bool FFlecsReplicationInterestBinding::operator==(
	const FFlecsReplicationInterestBinding& Other) const
{
	return PolicyName == Other.PolicyName && Descriptor == Other.Descriptor;
}

bool FFlecsReplicationRouteDescriptor::operator==(
	const FFlecsReplicationRouteDescriptor& Other) const
{
	return LogicalKey == Other.LogicalKey
		&& Interest == Other.Interest
		&& PollFrequency == Other.PollFrequency
		&& StaticPriority == Other.StaticPriority
		&& SchedulerWeight == Other.SchedulerWeight
		&& PageEntityLimit == Other.PageEntityLimit
		&& PageByteLimit == Other.PageByteLimit;
}

namespace
{
	const FFlecsComponentReplicationDescriptor* GetPairStorageDescriptor(
		const FFlecsComponentReplicationRegistry& Registry, const FFlecsId First, const FFlecsId Second)
	{
		const FFlecsComponentReplicationDescriptor* FirstDescriptor = Registry.Find(First);
		if (FirstDescriptor && !FirstDescriptor->bIsTag)
		{
			return FirstDescriptor;
		}
		
		const FFlecsComponentReplicationDescriptor* SecondDescriptor = Registry.Find(Second);
		if (SecondDescriptor && !SecondDescriptor->bIsTag)
		{
			return SecondDescriptor;
		}
		
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
	
	if (!Guid.IsValid())
	{
		Guid.D = 1;
	}
	
	return FFlecsReplicationLayoutId(Guid);
}

FString FFlecsReplicationKey::CanonicalString() const
{
	/*return FString::Printf(TEXT("%u|%s|%u|%s|%u|%u|%s|%u|%s|%llu|%u"),
		static_cast<uint8>(Kind), *RelationshipSchema.ToString(), RelationshipVersion,
		*StorageSchema.ToString(), StorageVersion, static_cast<uint8>(TargetKind),
		*TargetSchema.ToString(), TargetVersion, *StableTargetIdentifier,
		EntityTarget.GetValue(), bHasPayload ? 1u : 0u);*/
	return FString::Printf(TEXT("%u|%s|%s|%s|%s|%llu|%u|%s"),
		static_cast<uint8>(Kind), *RelationshipSchema.ToString(),
		*StorageSchema.ToString(), *TargetSchema.ToString(),
		*StableTargetIdentifier, EntityTarget.GetValue(), bHasPayload ? 1u : 0u,
		*CodecFingerprint);
}

void FFlecsReplicatedEntityUpdate::SetKeyChanged(const uint16 KeyIndex)
{
	const int32 WordIndex = KeyIndex / KeyMaskWordBits;
	if (ChangedKeyMask.Num() <= WordIndex)
	{
		ChangedKeyMask.SetNumZeroed(WordIndex + 1);
	}

	ChangedKeyMask[WordIndex] |= uint64(1) << (KeyIndex % KeyMaskWordBits);
}

bool FFlecsReplicatedEntityUpdate::IsKeyChanged(const uint16 KeyIndex) const
{
	const int32 WordIndex = KeyIndex / KeyMaskWordBits;
	return ChangedKeyMask.IsValidIndex(WordIndex)
		&& (ChangedKeyMask[WordIndex] & (uint64(1) << (KeyIndex % KeyMaskWordBits))) != 0;
}

uint32 FFlecsReplicatedEntityUpdate::GetPayloadByteCount() const
{
	uint32 Result = 0;
	for (const FFlecsReplicatedValue& Value : Values)
	{
		Result += Value.Bytes.Num();
	}
	return Result;
}

bool FFlecsReplicationUpdateChunk::IsKeyChanged(const uint16 InKeyIndex) const
{
	const int32 WordIndex = InKeyIndex / FFlecsReplicatedEntityUpdate::KeyMaskWordBits;
	return ChangedKeyMask.IsValidIndex(WordIndex)
		&& (ChangedKeyMask[WordIndex]
			& (uint64(1) << (InKeyIndex % FFlecsReplicatedEntityUpdate::KeyMaskWordBits))) != 0;
}

void BuildFlecsReplicationUpdateChunks(const FFlecsReplicatedEntityUpdate& Update,
	TArray<FFlecsReplicationUpdateChunk>& OutChunks)
{
	OutChunks.Reset();

	auto InitializeChunk = [&Update](FFlecsReplicationUpdateChunk& Chunk)
	{
		Chunk.NetworkId = Update.NetworkId;
		Chunk.StateRevision = Update.StateRevision;
		Chunk.CompositionRevision = Update.CompositionRevision;
		Chunk.Kind = Update.Kind;
		Chunk.LayoutId = Update.LayoutId;
		Chunk.Route = Update.Route;
		Chunk.ChangedKeyMask = Update.ChangedKeyMask;
		Chunk.ValueCount = static_cast<uint16>(Update.Values.Num());
	};

	FFlecsReplicationUpdateChunk& Header = OutChunks.AddDefaulted_GetRef();
	InitializeChunk(Header);

	for (const FFlecsReplicatedValue& Value : Update.Values)
	{
		const uint32 TotalBytes = Value.Bytes.Num();
		if (TotalBytes == 0)
		{
			FFlecsReplicationUpdateChunk& Chunk = OutChunks.AddDefaulted_GetRef();
			InitializeChunk(Chunk);
			Chunk.KeyIndex = Value.KeyIndex;
			continue;
		}

		for (uint32 Offset = 0; Offset < TotalBytes; Offset += FFlecsReplicationUpdateChunk::MaxChunkBytes)
		{
			FFlecsReplicationUpdateChunk& Chunk = OutChunks.AddDefaulted_GetRef();
			InitializeChunk(Chunk);
			Chunk.KeyIndex = Value.KeyIndex;
			Chunk.Offset = Offset;
			Chunk.TotalValueBytes = TotalBytes;
			const uint32 ByteCount = FMath::Min(FFlecsReplicationUpdateChunk::MaxChunkBytes, TotalBytes - Offset);
			Chunk.Bytes.Append(Value.Bytes.GetData() + Offset, ByteCount);
		}
	}
	INC_DWORD_STAT_BY(STAT_FlecsReplicationChunks, OutChunks.Num());
}

bool FFlecsReplicationUpdateReassembler::Accept(const FGuid& SourceShard,
	const FFlecsReplicationUpdateChunk& Chunk, TOptional<FFlecsReplicatedEntityUpdate>& OutUpdate, FString& OutError)
{
	OutUpdate.Reset();
	if (!SourceShard.IsValid() || !Chunk.NetworkId.IsValid() || !Chunk.LayoutId.IsValid()
		|| Chunk.StateRevision == 0 || Chunk.Bytes.Num() > FFlecsReplicationUpdateChunk::MaxChunkBytes
		|| (Chunk.Kind != EFlecsReplicatedEntityUpdateKind::Full
			&& Chunk.Kind != EFlecsReplicatedEntityUpdateKind::Delta))
	{
		OutError = TEXT("Received invalid Flecs replication chunk metadata");
		return false;
	}
	if ((Chunk.KeyIndex == FFlecsReplicationUpdateChunk::HeaderKeyIndex
			&& (!Chunk.Bytes.IsEmpty() || Chunk.Offset != 0 || Chunk.TotalValueBytes != 0))
		|| (Chunk.KeyIndex != FFlecsReplicationUpdateChunk::HeaderKeyIndex
			&& (!Chunk.IsKeyChanged(Chunk.KeyIndex)
				|| (Chunk.TotalValueBytes == 0 && (Chunk.Offset != 0 || !Chunk.Bytes.IsEmpty()))
				|| (Chunk.TotalValueBytes != 0 && Chunk.Bytes.IsEmpty()))))
	{
		OutError = TEXT("Received invalid Flecs replication chunk payload metadata");
		return false;
	}

	const FKey Key{ SourceShard, Chunk.NetworkId };
	if (const uint32* LatestCompleted = LatestCompletedRevisions.Find(Key);
		LatestCompleted && *LatestCompleted >= Chunk.StateRevision)
	{
		return true;
	}
	FAssembly* Existing = Assemblies.Find(Key);
	if (Existing && Existing->Update.StateRevision > Chunk.StateRevision)
	{
		return true;
	}

	if (!Existing || Existing->Update.StateRevision < Chunk.StateRevision)
	{
		FAssembly Assembly;
		Assembly.Update.NetworkId = Chunk.NetworkId;
		Assembly.Update.StateRevision = Chunk.StateRevision;
		Assembly.Update.CompositionRevision = Chunk.CompositionRevision;
		Assembly.Update.Kind = Chunk.Kind;
		Assembly.Update.LayoutId = Chunk.LayoutId;
		Assembly.Update.Route = Chunk.Route;
		Assembly.Update.ChangedKeyMask = Chunk.ChangedKeyMask;
		Existing = &Assemblies.Add(Key, MoveTemp(Assembly));
	}

	FAssembly& Assembly = *Existing;
	if (Assembly.Update.CompositionRevision != Chunk.CompositionRevision
		|| Assembly.Update.Kind != Chunk.Kind || Assembly.Update.LayoutId != Chunk.LayoutId
		|| Assembly.Update.Route != Chunk.Route || Assembly.Update.ChangedKeyMask != Chunk.ChangedKeyMask)
	{
		OutError = TEXT("Received inconsistent Flecs replication chunks for one revision");
		Assemblies.Remove(Key);
		return false;
	}
	if (Assembly.bReceivedChunkMetadata && Assembly.ExpectedValueCount != Chunk.ValueCount)
	{
		OutError = TEXT("Received inconsistent Flecs replication chunk value count");
		Assemblies.Remove(Key);
		return false;
	}
	Assembly.bReceivedChunkMetadata = true;
	Assembly.ExpectedValueCount = Chunk.ValueCount;

	if (Chunk.KeyIndex == FFlecsReplicationUpdateChunk::HeaderKeyIndex)
	{
		if (Assembly.bReceivedHeader)
		{
			OutError = TEXT("Received duplicate Flecs replication update header");
			Assemblies.Remove(Key);
			return false;
		}
		Assembly.bReceivedHeader = true;
	}
	else
	{
		if (Chunk.Offset > Chunk.TotalValueBytes
			|| static_cast<uint64>(Chunk.Offset) + Chunk.Bytes.Num() > Chunk.TotalValueBytes)
		{
			OutError = TEXT("Received out-of-bounds Flecs replication chunk");
			Assemblies.Remove(Key);
			return false;
		}

		FValueAssembly& Value = Assembly.Values.FindOrAdd(Chunk.KeyIndex);
		if (Chunk.TotalValueBytes == 0)
		{
			if (Value.bReceivedZeroLength)
			{
				OutError = TEXT("Received duplicate zero-length Flecs replication value chunk");
				Assemblies.Remove(Key);
				return false;
			}
			Value.bReceivedZeroLength = true;
		}
		else if (Value.Bytes.IsEmpty())
		{
			Value.TotalBytes = Chunk.TotalValueBytes;
			Value.Bytes.SetNumZeroed(Value.TotalBytes);
			Value.ReceivedBytes.Init(false, Value.TotalBytes);
		}
		else if (Value.TotalBytes != Chunk.TotalValueBytes)
		{
			OutError = TEXT("Received inconsistent Flecs replication value length");
			Assemblies.Remove(Key);
			return false;
		}

		for (int32 ByteIndex = 0; ByteIndex < Chunk.Bytes.Num(); ++ByteIndex)
		{
			const uint32 Destination = Chunk.Offset + ByteIndex;
			if (Value.ReceivedBytes[Destination])
			{
				OutError = TEXT("Received overlapping Flecs replication chunks");
				Assemblies.Remove(Key);
				return false;
			}
			Value.ReceivedBytes[Destination] = true;
			Value.Bytes[Destination] = Chunk.Bytes[ByteIndex];
			++Value.ReceivedCount;
		}
		if (Assembly.Values.Num() > Assembly.ExpectedValueCount)
		{
			OutError = TEXT("Received too many Flecs replication values for one update");
			Assemblies.Remove(Key);
			return false;
		}
	}

	if (!Assembly.bReceivedHeader || Assembly.Values.Num() != Assembly.ExpectedValueCount)
	{
		return true;
	}

	for (const TPair<uint16, FValueAssembly>& Pair : Assembly.Values)
	{
		if (Pair.Value.ReceivedCount != Pair.Value.TotalBytes)
		{
			return true;
		}
	}

	Assembly.Values.KeySort([](const uint16 A, const uint16 B) { return A < B; });
	for (TPair<uint16, FValueAssembly>& Pair : Assembly.Values)
	{
		FFlecsReplicatedValue& Value = Assembly.Update.Values.AddDefaulted_GetRef();
		Value.KeyIndex = Pair.Key;
		Value.Bytes = MoveTemp(Pair.Value.Bytes);
	}

	OutUpdate = MoveTemp(Assembly.Update);
	LatestCompletedRevisions.Add(Key, Chunk.StateRevision);
	Assemblies.Remove(Key);
	return true;
}

void FFlecsReplicationUpdateReassembler::RemoveSource(const FGuid& SourceShard)
{
	for (auto It = Assemblies.CreateIterator(); It; ++It)
	{
		if (It.Key().SourceShard == SourceShard)
		{
			It.RemoveCurrent();
		}
	}
	for (auto It = LatestCompletedRevisions.CreateIterator(); It; ++It)
	{
		if (It.Key().SourceShard == SourceShard)
		{
			It.RemoveCurrent();
		}
	}
}

void FFlecsReplicationUpdateReassembler::RemoveEntity(const FGuid& SourceShard, const FFlecsNetworkId NetworkId)
{
	Assemblies.Remove(FKey{ SourceShard, NetworkId });
	LatestCompletedRevisions.Remove(FKey{ SourceShard, NetworkId });
}

bool FFlecsReplicationUpdateReassembler::ReferencesLayout(const FFlecsReplicationLayoutId LayoutId) const
{
	for (const TPair<FKey, FAssembly>& Pair : Assemblies)
	{
		if (Pair.Value.Update.LayoutId == LayoutId)
		{
			return true;
		}
	}
	return false;
}

void FFlecsReplicationUpdateReassembler::Reset()
{
	Assemblies.Reset();
	LatestCompletedRevisions.Reset();
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

	const FFlecsComponentReplicationRegistry& Registry = FFlecsComponentReplicationRegistry::Get(World);
	TArray<FFlecsReplicationKey> Keys;
	TSet<FFlecsId> SeenIds;
	for (const FFlecsId Id : Entity.GetType())
	{
		SeenIds.Add(Id);
		if (!Id.IsPair())
		{
			const FFlecsComponentReplicationDescriptor* Descriptor = Registry.Find(Id);
			
			if (!Descriptor)
			{
				continue;
			}
			
			FFlecsReplicationKey& Key = Keys.AddDefaulted_GetRef();
			Key.Kind = EFlecsReplicationKeyKind::Component;
			Key.StorageSchema = Descriptor->SchemaId;
			Key.bHasPayload = !Descriptor->bIsTag;
			Key.CodecFingerprint = Descriptor->CodecFingerprint;
			continue;
		}

		// Pair storage omits entity generations from both elements. Restore the
		// current alive IDs before using them as local registry keys.
		const FFlecsEntityHandle RelationshipEntity = World->GetAlive(Id.GetFirst());
		const FFlecsEntityHandle Target = World->GetAlive(Id.GetSecond());
		const FFlecsId First = RelationshipEntity.IsValid()
			? RelationshipEntity.GetFlecsId()
			: Id.GetFirst();
		const FFlecsId Second = Target.IsValid()
			? Target.GetFlecsId()
			: Id.GetSecond();
		
		const FFlecsComponentReplicationDescriptor* Relationship = Registry.Find(First);
		const FFlecsComponentReplicationDescriptor* Storage = GetPairStorageDescriptor(Registry, First, Second);
		
		if (!Relationship || !Storage)
		{
			continue;
		}

		FFlecsReplicationKey& Key = Keys.AddDefaulted_GetRef();
		Key.Kind = EFlecsReplicationKeyKind::Pair;
		Key.RelationshipSchema = Relationship->SchemaId;
		Key.StorageSchema = Storage->SchemaId;
		Key.bHasPayload = !Storage->bIsTag;
		Key.CodecFingerprint = Storage->CodecFingerprint;

		const FFlecsComponentReplicationDescriptor* TargetDescriptor = Registry.Find(Second);
		if (TargetDescriptor)
		{
			Key.TargetKind = EFlecsReplicationPairTargetKind::Schema;
			Key.TargetSchema = TargetDescriptor->SchemaId;
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
				Key.TargetKind = EFlecsReplicationPairTargetKind::Entity;
				Key.EntityTarget = Target.Get<FFlecsNetworkId>();
			}
			else if (Target.HasSymbol())
			{
				Key.TargetKind = EFlecsReplicationPairTargetKind::StableSymbolValue;
				Key.StableTargetIdentifier = Target.GetSymbol();
			}
			else if (Target.Has<FFlecsStablePathTag>() && Target.HasName())
			{
				Key.TargetKind = EFlecsReplicationPairTargetKind::StablePathValue;
				Key.StableTargetIdentifier = Target.GetPath();
			}
			else UNLIKELY_ATTRIBUTE
			{
				OutError = FString::Printf(TEXT("Cannot build a replication layout for a pair with an unknown target: %s"), 
					*Id.ToString());
				return nullptr;
			}
		}
	}

	// DontFragment components are owned outside the entity's table type. Probe
	// registered replicated IDs so two entities sharing a table can still have
	// distinct authoritative layouts.
	for (const TPair<FFlecsId, FFlecsComponentReplicationDescriptor>& Pair : Registry.GetDescriptors())
	{
		if (SeenIds.Contains(Pair.Key) || !Entity.Has(Pair.Key))
		{
			continue;
		}
		FFlecsReplicationKey& Key = Keys.AddDefaulted_GetRef();
		Key.Kind = EFlecsReplicationKeyKind::Component;
		Key.StorageSchema = Pair.Value.SchemaId;
		Key.bHasPayload = !Pair.Value.bIsTag;
		Key.CodecFingerprint = Pair.Value.CodecFingerprint;
	}
	if (Keys.Num() > MAX_uint16)
	{
		OutError = TEXT("Replication layouts cannot contain more than 65535 keys");
		return nullptr;
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
		return Existing;
	}

	const FFlecsReplicationLayoutId Id = Definition.LayoutId;
	Definitions.Add(Id, MoveTemp(Definition));
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
	if (!Definition.LayoutId.IsValid() || Definition.Keys.Num() > MAX_uint16
		|| ComputeLayoutId(Definition.Keys) != Definition.LayoutId)
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

void FFlecsReplicationLayoutRegistry::RemoveDefinition(const FFlecsReplicationLayoutId Id)
{
	Definitions.Remove(Id);
}
