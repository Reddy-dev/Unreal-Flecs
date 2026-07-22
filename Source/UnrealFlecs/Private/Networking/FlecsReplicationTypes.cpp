// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/FlecsReplicationTypes.h"

#include "Misc/SecureHash.h"
#include "Networking/FlecsReplicatedTrait.h"
#include "Networking/FlecsStablePathTag.h"
#include "Serialization/MemoryWriter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsReplicationTypes)

const FName FFlecsReplicationInterestPolicyNames::Everyone(TEXT("Everyone"));
const FName FFlecsReplicationInterestPolicyNames::Owner(TEXT("Owner"));
const FName FFlecsReplicationInterestPolicyNames::SpatialCell(TEXT("SpatialCell"));

FFlecsReplicationInterestBinding FFlecsReplicationInterestBinding::Everyone()
{
	return Make(FFlecsReplicationInterestPolicyNames::Everyone,
		FFlecsReplicationEveryoneInterestDescriptor{});
}

FFlecsReplicationRouteDescriptor::FFlecsReplicationRouteDescriptor()
	: Interest(FFlecsReplicationInterestBinding::Everyone())
{
}

FFlecsReplicationRouteDescriptor FFlecsReplicationRouteDescriptor::Default()
{
	return {};
}

FIntVector FlecsReplicationSpatialCell(const FVector& Position, const float CellSize)
{
	if (CellSize <= UE_SMALL_NUMBER)
	{
		return FIntVector::ZeroValue;
	}

	return FIntVector(FMath::FloorToInt(Position.X / CellSize),
		FMath::FloorToInt(Position.Y / CellSize), FMath::FloorToInt(Position.Z / CellSize));
}

FFlecsReplicationRouteDescriptor MakeFlecsSpatialCellRoute(const FVector& Position,
	const float CellSize, const int32 SpatialLayer, const float BubbleRadius, const FName LogicalRoute)
{
	FFlecsReplicationSpatialCellInterestDescriptor Descriptor;
	Descriptor.Cell = FlecsReplicationSpatialCell(Position, CellSize);
	Descriptor.CellSize = CellSize;
	Descriptor.SpatialLayer = SpatialLayer;
	Descriptor.BubbleRadius = BubbleRadius;

	FFlecsReplicationRouteDescriptor Result = FFlecsReplicationRouteDescriptor::Default();
	Result.LogicalKey = FFlecsReplicationRouteKey(LogicalRoute);
	Result.Interest = FFlecsReplicationInterestBinding::Make(
		FFlecsReplicationInterestPolicyNames::SpatialCell, Descriptor);
	return Result;
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

bool FFlecsReplicatedEntityUpdate::IsKeyChanged(const uint16 KeyIndex) const
{
	return ChangedKeys.Contains(KeyIndex);
}

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

	NO_DISCARD bool IsReplicationStructureEligible(const TSolidNotNull<const UFlecsWorld*> World,
		const FFlecsComponentReplicationRegistry& Registry, const FFlecsId Id)
	{
		if (Registry.Find(Id))
		{
			return true;
		}

		// Symbols and stable paths describe an eligible ID; they do not opt
		// every named Flecs component into replicated composition.
		const FFlecsEntityHandle IdEntity = World->GetAlive(Id);
		return IdEntity.IsValid()
			&& (IdEntity.Has<FFlecsNetworkId>() || IdEntity.Has<FFlecsReplicatedTrait>());
	}

	NO_DISCARD bool TryBuildIndividualKey(const TSolidNotNull<const UFlecsWorld*> World,
		const FFlecsComponentReplicationRegistry& Registry, const FFlecsId Id, FFlecsReplicationIndividualKey& OutKey)
	{
		OutKey = {};

		if (const FFlecsComponentReplicationDescriptor* Descriptor = Registry.Find(Id))
		{
			OutKey.Kind = EFlecsReplicationPairTargetKind::Schema;
			OutKey.Schema = Descriptor->SchemaId;
			return true;
		}

		const FFlecsEntityHandle IdEntity = World->GetAlive(Id);
		if (!IdEntity.IsValid())
		{
			return false;
		}

		if (IdEntity.Has<FFlecsNetworkId>())
		{
			OutKey.Kind = EFlecsReplicationPairTargetKind::Entity;
			OutKey.Entity = IdEntity.Get<FFlecsNetworkId>();
			return true;
		}

		if (IdEntity.HasSymbol())
		{
			OutKey.Kind = EFlecsReplicationPairTargetKind::StableSymbolValue;
			OutKey.StableIdentifier = IdEntity.GetSymbol();
			return true;
		}

		if (IdEntity.Has<FFlecsStablePathTag>() && IdEntity.HasName())
		{
			OutKey.Kind = EFlecsReplicationPairTargetKind::StablePathValue;
			OutKey.StableIdentifier = IdEntity.GetPath();
			return true;
		}

		return false;
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
                                                                                         const FFlecsEntityHandle& Entity, 
                                                                                         bool& bOutWasCreated, FString& OutError)
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
			if (!IsReplicationStructureEligible(World, Registry, Id))
			{
				continue;
			}

			if UNLIKELY_IF(!TryBuildIndividualKey(World, Registry, Id, Key.Primary))
			{
				OutError = FString::Printf(TEXT("Cannot encode replicated component ID: %s"), *Id.ToString());
				return nullptr;
			}

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
		
		const EFlecsReplicationKeyStorageKind StorageKind = GetStorageKindForPair(Registry, First, Second);

		if (StorageKind == EFlecsReplicationKeyStorageKind::None
			&& !IsReplicationStructureEligible(World, Registry, First))
		{
			continue;
		}

		if UNLIKELY_IF(!TryBuildIndividualKey(World, Registry, First, Key.Primary)
			|| !TryBuildIndividualKey(World, Registry, Second, Key.Secondary))
		{
			OutError = FString::Printf(TEXT("Cannot encode replicated pair ID: %s"), *Id.ToString());
			return nullptr;
		}

		Key.Kind = EFlecsReplicationKeyKind::Pair;
		Key.StorageKind = StorageKind;
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

void FFlecsReplicationLayoutRegistry::HandleTableDestruction(const TSolidNotNull<const UFlecsWorld*> World)
{
	ecs_table
}
