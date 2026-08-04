// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Shards/FlecsNetEntityTable.h"

#include "Net/UnrealNetwork.h"

#include "Networking/Shards/FlecsNetEntityTableNetFactory.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsNetEntityTable)

void UFlecsNetEntityTable::PostInitProperties()
{
	Super::PostInitProperties();

	EntityTable.SetOwner(this);
}

void UFlecsNetEntityTable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams LifetimeParams;
	LifetimeParams.bIsPushBased = false;
	DOREPLIFETIME_WITH_PARAMS_FAST(UFlecsNetEntityTable, EntityTable, LifetimeParams);
}

void UFlecsNetEntityTable::ConfigureObjectSettings(OUT UE::Net::FRootObjectSettings& OutSettings) const
{
	Super::ConfigureObjectSettings(OutSettings);

	OutSettings.FactoryName = UFlecsNetEntityTableNetFactory::GetFactoryName();
}

bool UFlecsNetEntityTable::CanAcceptNetEntity(const FFlecsNetworkId& InNetworkId,
	const FFlecsEntityReplicationSnapshot&) const
{
	return InNetworkId.IsValid();
}

void UFlecsNetEntityTable::PublishNetEntity(const FFlecsNetworkId& InNetworkId,
	const FFlecsEntityReplicationSnapshot& InSnapshot)
{
	solid_checkf(CanAcceptNetEntity(InNetworkId, InSnapshot),
		TEXT("Cannot publish invalid network ID to Flecs entity table '%s'"), *GetName());

	if (FFlecsNetEntityTableItem* ExistingItem = EntityTable.Items.FindByPredicate(
		[&InNetworkId](const FFlecsNetEntityTableItem& Item)
		{
			return Item.NetworkId == InNetworkId;
		}))
	{
		ExistingItem->Snapshot = InSnapshot;
		EntityTable.MarkItemDirty(*ExistingItem);
		return;
	}

	FFlecsNetEntityTableItem& NewItem = EntityTable.Items.Emplace_GetRef();
	NewItem.NetworkId = InNetworkId;
	NewItem.Snapshot = InSnapshot;
	EntityTable.MarkItemDirty(NewItem);
}

void UFlecsNetEntityTable::RemoveNetEntity(const FFlecsNetworkId& InNetworkId)
{
	const int32 RemovedIndex = EntityTable.Items.IndexOfByPredicate(
		[&InNetworkId](const FFlecsNetEntityTableItem& Item)
		{
			return Item.NetworkId == InNetworkId;
		});
	solid_checkf(RemovedIndex != INDEX_NONE,
		TEXT("Cannot remove network ID '%s' from Flecs entity table '%s'"),
		*InNetworkId.ToString(), *GetName());

	EntityTable.Items.RemoveAt(RemovedIndex);
	EntityTable.MarkArrayDirty();
}

bool UFlecsNetEntityTable::IsEmpty() const
{
	return EntityTable.Items.IsEmpty();
}

void UFlecsNetEntityTable::HandleReplicationDetached()
{
	for (const FFlecsNetEntityTableItem& Item : EntityTable.Items)
	{
		ReceiveEntityRemoval(Item.NetworkId, Item.Snapshot.StateRevision);
	}
}

void UFlecsNetEntityTable::HandleEntityRemoved(const FFlecsNetworkId& InNetworkId, const uint32 InStateRevision)
{
	ReceiveEntityRemoval(InNetworkId, InStateRevision);
}

void UFlecsNetEntityTable::HandleEntityUpdated(const FFlecsNetworkId& InNetworkId,
	const FFlecsEntityReplicationSnapshot& InSnapshot)
{
	ReceiveEntityUpdate(InNetworkId, InSnapshot);
}

void UFlecsNetEntityTable::OnRep_EntityTable()
{
}
