// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Layout/FlecsLayoutReplicatorFastArray.h"

#include "Networking/Layout/FlecsLayoutReplicator.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsLayoutReplicatorFastArray)

void FFlecsLayoutReplicatorItem::PreReplicatedRemove(const FFlecsReplicatorFastArray& InArraySerializer)
{
	InArraySerializer.ReceiveLayoutRemoval(LayoutDefinition.LayoutId);
}

void FFlecsLayoutReplicatorItem::PostReplicatedAdd(const FFlecsReplicatorFastArray& InArraySerializer)
{
	InArraySerializer.ReceiveLayout(LayoutDefinition);
}

void FFlecsLayoutReplicatorItem::PostReplicatedChange(const FFlecsReplicatorFastArray& InArraySerializer)
{
	InArraySerializer.ReceiveLayout(LayoutDefinition);
}

bool FFlecsReplicatorFastArray::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FFlecsLayoutReplicatorItem, FFlecsReplicatorFastArray>(
		Items, DeltaParms, *this);
}

void FFlecsReplicatorFastArray::SetOwner(UFlecsLayoutReplicator* InOwner)
{
	Owner = InOwner;
}

bool FFlecsReplicatorFastArray::AddLayout(const FFlecsReplicationLayoutDefinition& InLayoutDefinition)
{
	if (FFlecsLayoutReplicatorItem* ExistingItem = Items.FindByPredicate(
		[&InLayoutDefinition](const FFlecsLayoutReplicatorItem& Item)
		{
			return Item.LayoutDefinition.LayoutId == InLayoutDefinition.LayoutId;
		}))
	{
		if (ExistingItem->LayoutDefinition.Keys == InLayoutDefinition.Keys)
		{
			return false;
		}

		UE_LOG(LogFlecsCore, Error,
			TEXT("Cannot replace immutable Flecs layout '%s' with a different definition"),
			*InLayoutDefinition.LayoutId.ToString());
		return false;
	}

	FFlecsLayoutReplicatorItem& NewItem = Items.AddDefaulted_GetRef();
	NewItem.LayoutDefinition = InLayoutDefinition;
	MarkItemDirty(NewItem);
	return true;
}

bool FFlecsReplicatorFastArray::RemoveLayout(const FFlecsReplicationLayoutId& InLayoutId)
{
	const int32 ItemIndex = Items.IndexOfByPredicate(
		[&InLayoutId](const FFlecsLayoutReplicatorItem& Item)
		{
			return Item.LayoutDefinition.LayoutId == InLayoutId;
		});

	if (ItemIndex == INDEX_NONE)
	{
		return false;
	}

	Items.RemoveAtSwap(ItemIndex, EAllowShrinking::No);
	MarkArrayDirty();
	return true;
}

const FFlecsLayoutReplicatorItem* FFlecsReplicatorFastArray::FindLayout(
	const FFlecsReplicationLayoutId& InLayoutId) const
{
	return Items.FindByPredicate(
		[&InLayoutId](const FFlecsLayoutReplicatorItem& Item)
		{
			return Item.LayoutDefinition.LayoutId == InLayoutId;
		});
}

void FFlecsReplicatorFastArray::ReceiveLayout(
	const FFlecsReplicationLayoutDefinition& InLayoutDefinition) const
{
	if (Owner)
	{
		Owner->ReceiveLayout(InLayoutDefinition);
	}
}

void FFlecsReplicatorFastArray::ReceiveLayoutRemoval(const FFlecsReplicationLayoutId& InLayoutId) const
{
	if (Owner)
	{
		Owner->ReceiveLayoutRemoval(InLayoutId);
	}
}
