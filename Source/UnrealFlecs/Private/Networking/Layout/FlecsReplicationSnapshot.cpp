// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Layout/FlecsReplicationSnapshot.h"

#include "Serialization/MemoryWriter.h"

#include "Networking/FlecsNetworkWorldSubsystem.h"
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
		
		FFlecsReplicatedValue Value;
		Value.KeyIndex = Index;
		//Value.bDontFragment = false;
		
		if (!FFlecsReplicationKey::IsValidPairStorageKind(Key.StorageKind))
		{
			continue;
		}
		
		const TSolidNotNull<const FFlecsComponentReplicationDescriptor*> Descriptor = ComponentRegistry.Find(ComponentId);
		
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
		
		if UNLIKELY_IF(!Descriptor->GetSerializeFunction()(Writer, const_cast<void*>(ComponentValuePtr)) || Writer.IsError())
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
	
	/*TArray<FFlecsReplicationKey> DontFragmentKeys;
	
	InNetworkWorldSubsystem->IterateDontFragmentOnEntity(InEntityHandle,
		[this, World, &DontFragmentKeys](const flecs::iter& InIter, size_t InIndex)
	{
		// Should be the component on our $this entity
		FFlecsEntityHandle ComponentEntity = InIter.entity(InIndex);
			
		TValueOrError<FFlecsReplicationKey, FString> Key = FFlecsReplicationKey::BuildKey(World, ComponentEntity);
		solid_checkf(Key.IsValid(), TEXT("Failed to build replication key for component entity %s"), *ComponentEntity.ToString());
			
		DontFragmentKeys.Add(Key.GetValue());
	});
	
	DontFragmentKeys.Sort([](const FFlecsReplicationKey& A, const FFlecsReplicationKey& B)
	{
		return A.CanonicalString() < B.CanonicalString();
	});
	
	const uint32 DontFragmentKeyCount = DontFragmentKeys.Num();
	
	for (uint32 Index = 0; Index < DontFragmentKeyCount; ++Index)
	{
		const FFlecsReplicationKey& Key = DontFragmentKeys[Index];
		
		const FFlecsId ComponentId = FFlecsReplicationKey::ResolveToId(World, Key);
		if UNLIKELY_IF(!ComponentId.IsValid())
		{
			UE_LOGFMT(LogFlecsCore, Error, 
				"Failed to resolve component ID for key {Key} when marking snapshot values as DontFragment for entity {Entity}.", 
				Key.CanonicalString(), *InEntityHandle.ToString());
			continue;
		}
		
		const void* ComponentValuePtr = InEntityHandle.TryGet(ComponentId);
		
		if UNLIKELY_IF(!ComponentValuePtr)
		{
			UE_LOGFMT(LogFlecsCore, Error, 
				"Failed to get component value for component ID {ComponentId} when marking snapshot values as DontFragment for entity {Entity}.", 
				ComponentId.ToString(), *InEntityHandle.ToString());
			continue;
		}
		
		
	}
	*/
	
	++StateRevision;
}
