// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Layout/FlecsReplicationSnapshot.h"

#include "Serialization/MemoryWriter.h"

#include "Networking/Subsystem/FlecsNetworkWorldSubsystem.h"
#include "Networking/Layout/FlecsReplicationLayoutDefinition.h"
#include "Networking/Layout/FlecsReplicationLayoutRegistry.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsReplicationSnapshot)

void FFlecsEntityReplicationSnapshot::FillFromEntity(const FFlecsEntityHandle& InEntityHandle, 
	const FFlecsReplicationLayoutRegistry& InLayoutRegistry)
{
	solid_checkf(InEntityHandle.IsValid(), TEXT("Cannot fill snapshot from invalid entity handle"));
	
	const TSolidNotNull<UFlecsWorldInterfaceObject*> World = InEntityHandle.GetFlecsWorld();
	
	const FFlecsComponentReplicationRegistry& ComponentRegistry = FFlecsComponentReplicationRegistry::Get(World->GetFlecsWorld());
	
	const FFlecsReplicationLayoutDefinition* LayoutDefinition = InLayoutRegistry.Find(LayoutId);
	if UNLIKELY_IF (!LayoutDefinition)
	{
		UE_LOGFMT(LogFlecsCore, Error, 
			"Failed to find layout definition for layout ID {Layout} when filling snapshot for entity {Entity}.", 
			LayoutId.ToString(), *InEntityHandle.ToString());
		return;
	}
	
	Values.Empty();
	
	const uint32 KeyCount = LayoutDefinition->Keys.Num();
	
	for (uint32 Index = 0; Index < KeyCount; ++Index)
	{
		const FFlecsReplicationKey& Key = LayoutDefinition->Keys[Index];
		
		const FFlecsId ComponentId = FFlecsReplicationKey::ResolveToId(World, Key);
		if UNLIKELY_IF(!ComponentId.IsValid())
		{
			UE_LOGFMT(LogFlecsCore, Error, 
				"Failed to resolve component ID for key {Key} when filling snapshot for entity {Entity}.", 
				Key.CanonicalString(), *InEntityHandle.ToString());
			continue;
		}
		
#if DO_CHECK
		
		if (ComponentId.IsPair())
		{
			const FFlecsId FirstId = ComponentId.GetFirst();
			const FFlecsId SecondId = ComponentId.GetSecond();
			
			solid_checkf(World->IsAlive(FirstId), 
				TEXT("Component ID %s is not alive when filling snapshot for entity %s."), 
				*FirstId.ToString(), *InEntityHandle.ToString());
			
			solid_checkf(World->IsAlive(SecondId), 
				TEXT("Component ID %s is not alive when filling snapshot for entity %s."), 
				*SecondId.ToString(), *InEntityHandle.ToString());
		}
		else
		{
			solid_checkf(World->IsAlive(ComponentId), 
				TEXT("Component ID %s is not alive when filling snapshot for entity %s."), 
				*ComponentId.ToString(), *InEntityHandle.ToString());
		}
		
#endif // DO_CHECK
		
		FFlecsReplicatedValue Value;
		Value.KeyIndex = Index;
		
		if (!FFlecsReplicationKey::IsValidPairStorageKind(Key.StorageKind))
		{
			continue;
		}
		
		const ecs_type_info_t* ValueTypeInfo = ComponentId.GetTypeInfo(World);
		solid_cassume(ValueTypeInfo);
		
		const FFlecsId ValueId = FFlecsId(ValueTypeInfo->component);
		solid_check(ValueId.IsValid());
		solid_checkf(ComponentRegistry.Find(ValueId), 
			TEXT("Component ID %s does not have a replication descriptor when filling snapshot for entity %s."), 
			*ValueId.ToString(), *InEntityHandle.ToString());
		
		const TSolidNotNull<const FFlecsComponentReplicationDescriptor*> Descriptor = ComponentRegistry.Find(ValueId);
		
		const void* ComponentValuePtr = InEntityHandle.TryGet(ComponentId);
		if UNLIKELY_IF(!ComponentValuePtr)
		{
			UE_LOGFMT(LogFlecsCore, Error, 
				"Failed to get component value for component ID {ComponentId} when filling snapshot for entity {Entity}.", 
				ComponentId.ToString(), *InEntityHandle.ToString());
			continue;
		}
		
		Value.Bytes.Reset();
		FMemoryWriter Writer(Value.Bytes, true);
		
		const bool bSerializeSuccess = Descriptor->GetSerializeFunction()(Writer, const_cast<void*>(ComponentValuePtr));
		
		if (!bSerializeSuccess || Writer.IsError())
		{
			UE_LOGFMT(LogFlecsCore, Error,
					"Failed to serialize component ID {ComponentId}.",
					ComponentId.ToString());
			continue;
		}
		
		Values.Add(MoveTemp(Value));
		solid_checkf(Values.Num() == Index + 1, 
			TEXT("Values array size mismatch after adding value for key %s"), 
			*Key.CanonicalString());
	}
	
	++StateRevision;
}

/*
namespace UE::Net
{
	UE_NET_IMPLEMENT_SERIALIZER(FFlecsEntityReplicationSnapshotSerializerConfig);
} // namespace UE::Net*/
