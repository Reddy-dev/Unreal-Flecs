// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/FlecsComponentReplicationDescriptor.h"

#include "Misc/SecureHash.h"
#include "UObject/UnrealType.h"

#include "Logs/FlecsCategories.h"
#include "Networking/FlecsNetworkId.h"
#include "Networking/FlecsReplicatedTrait.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsComponentReplicationDescriptor)

namespace
{
	using FWorldRegistryMap = TMap<TWeakObjectPtr<const UFlecsWorld>, TUniquePtr<FFlecsComponentReplicationRegistry>>;

	FWorldRegistryMap& GetWorldRegistries()
	{
		static FWorldRegistryMap Registries;
		return Registries;
	}

	void RemoveExpiredWorldRegistries()
	{
		for (auto It = GetWorldRegistries().CreateIterator(); It; ++It)
		{
			if (!It.Key().IsValid())
			{
				It.RemoveCurrent();
			}
		}
	}

	bool ValidateProperty(const FProperty* Property, TSet<const UStruct*>& Visited, FString& OutError)
	{
		if (Property->IsA<FSoftObjectProperty>() || Property->IsA<FSoftClassProperty>())
		{
			return true;
		}
		
		if (Property->IsA<FObjectPropertyBase>() || Property->IsA<FInterfaceProperty>())
		{
			OutError = FString::Printf(TEXT("Raw UObject reference property '%s' is not supported"), *Property->GetPathName());
			return false;
		}
		
		if (const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
		{
			return ValidateProperty(ArrayProperty->Inner, Visited, OutError);
		}
		
		if (const FSetProperty* SetProperty = CastField<FSetProperty>(Property))
		{
			return ValidateProperty(SetProperty->ElementProp, Visited, OutError);
		}
		
		if (const FMapProperty* MapProperty = CastField<FMapProperty>(Property))
		{
			return ValidateProperty(MapProperty->KeyProp, Visited, OutError)
				&& ValidateProperty(MapProperty->ValueProp, Visited, OutError);
		}
		
		if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
		{
			if (Visited.Contains(StructProperty->Struct))
			{
				return true;
			}
			
			Visited.Add(StructProperty->Struct);
			
			for (TFieldIterator<FProperty> It(StructProperty->Struct); It; ++It)
			{
				if (!ValidateProperty(*It, Visited, OutError))
				{
					return false;
				}
			}
		}
		
		return true;
	}
	
} // namespace

FFlecsReplicationSchemaId FFlecsReplicationSchemaId::FromStableName(const FString& StableName)
{
	if (StableName.IsEmpty())
	{
		return {};
	}

	const FTCHARToUTF8 Utf8(*StableName);
	
	FMD5 Md5;
	Md5.Update(reinterpret_cast<const uint8*>(Utf8.Get()), Utf8.Length());
	
	FMD5Hash Hash;
	Hash.Set(Md5);
	FGuid Guid = MD5HashToGuid(Hash);
	
	if (!Guid.IsValid())
	{
		Guid.D = 1;
	}
	
	return FFlecsReplicationSchemaId(Guid);
}

bool FFlecsComponentReplicationDescriptor::IsValid(FString* OutError) const
{
	auto Fail = [OutError](const TCHAR* Error)
	{
		if (OutError)
		{
			*OutError = Error;
		}
		
		return false;
	};
	
	if (StableName.IsEmpty() || !SchemaId.IsValid())
	{
		return Fail(TEXT("Replication stable name/schema ID is missing"));
	}
	
	if (!LocalFlecsId.IsValid())
	{
		return Fail(TEXT("Local Flecs ID is invalid"));
	}
	
	if (!bIsTag && (Size == 0 || Alignment == 0))
	{
		return Fail(TEXT("Data component size/alignment is invalid"));
	}
	
	if (!bIsTag && (!Serialize || !Deserialize || !Construct || !Destroy))
	{
		return Fail(TEXT("Native replication operations are incomplete"));
	}
	
	return true;
}

FFlecsComponentReplicationRegistry& FFlecsComponentReplicationRegistry::Get(const TSolidNotNull<const UFlecsWorld*> World)
{
	RemoveExpiredWorldRegistries();
	const TWeakObjectPtr<const UFlecsWorld> Key(World);
	
	TUniquePtr<FFlecsComponentReplicationRegistry>& Registry = GetWorldRegistries().FindOrAdd(Key);
	
	if (!Registry)
	{
		Registry = MakeUnique<FFlecsComponentReplicationRegistry>();
	}
	
	return *Registry;
}

void FFlecsComponentReplicationRegistry::RemoveWorld(const UFlecsWorld* World)
{
	if LIKELY_IF(World)
	{
		GetWorldRegistries().Remove(TWeakObjectPtr(World));
	}
}

bool FFlecsComponentReplicationRegistry::Register(const FFlecsComponentReplicationDescriptor& Descriptor, FString& OutError)
{
	if (!Descriptor.IsValid(&OutError))
	{
		return false;
	}
	
	if (Descriptor.ScriptStruct && !ValidateReflectedType(Descriptor.ScriptStruct, OutError))
	{
		return false;
	}
	
	if (const FFlecsId* ExistingLocal = SchemaToLocalId.Find(Descriptor.SchemaId))
	{
		if (*ExistingLocal == Descriptor.LocalFlecsId)
		{
			return true;
		}
		
		OutError = FString::Printf(TEXT("Duplicate replication schema ID %s for '%s'"),
		                           *Descriptor.SchemaId.ToString(), *Descriptor.StableName);
		return false;
	}
	
	const FFlecsId LocalId = Descriptor.LocalFlecsId;
	
	SchemaToLocalId.Add(Descriptor.SchemaId, LocalId);
	
	ByLocalId.Add(LocalId, Descriptor);
	DescriptorRegisteredDelegate.Broadcast(ByLocalId.FindChecked(LocalId));
	
	return true;
}

const FFlecsComponentReplicationDescriptor* FFlecsComponentReplicationRegistry::Find(const FFlecsId LocalId) const
{
	return ByLocalId.Find(LocalId);
}

const FFlecsComponentReplicationDescriptor* FFlecsComponentReplicationRegistry::Find(const FFlecsReplicationSchemaId& SchemaId) const
{
	const FFlecsId* LocalId = SchemaToLocalId.Find(SchemaId);
	return LocalId ? ByLocalId.Find(*LocalId) : nullptr;
}

bool FFlecsComponentReplicationRegistry::ValidateReflectedType(const TSolidNotNull<const UScriptStruct*> ScriptStruct, FString& OutError)
{
	TSet<const UStruct*> Visited;
	Visited.Add(ScriptStruct);
	
	for (TFieldIterator<FProperty> It(ScriptStruct); It; ++It)
	{
		if (!ValidateProperty(*It, Visited, OutError))
		{
			return false;
		}
	}
	
	return true;
}

bool FFlecsComponentReplicationRegistry::IsEntityReplicationEligible(const TSolidNotNull<const UFlecsWorld*> World,
	const FFlecsId Id)
{
	if UNLIKELY_IF(!Id.IsValid())
	{
		UE_LOGFMT(LogFlecsWorld, Error, 
			"Invalid Flecs ID provided for entity replication eligibility check in world");
		return false;
	}
	
	const FFlecsEntityHandle EntityHandle = World->GetAlive(Id);
	return EntityHandle.IsValid() && (EntityHandle.Has<FFlecsNetworkId>() || EntityHandle.Has<FFlecsReplicatedTrait>());
}

bool UE::Flecs::Replication::RegisterComponentDefinition(
	const TSolidNotNull<const UFlecsWorld*> InWorld,
	const FFlecsReplicationComponentDefinition& InDefinition,
	OUT FString* OutError)
{
	FFlecsComponentReplicationDescriptor Descriptor;
	Descriptor.StableName = InDefinition.StableName;
	Descriptor.SchemaId = FFlecsReplicationSchemaId::FromStableName(InDefinition.StableName);
	Descriptor.LocalFlecsId = InDefinition.LocalFlecsId;
	Descriptor.Size = InDefinition.Size;
	Descriptor.Alignment = InDefinition.Alignment;
	Descriptor.bIsTag = InDefinition.bIsTag;
	Descriptor.ScriptStruct = InDefinition.ScriptStruct;
	Descriptor.Serialize = InDefinition.Serialize;
	Descriptor.Deserialize = InDefinition.Deserialize;
	Descriptor.Construct = InDefinition.Construct;
	Descriptor.Destroy = InDefinition.Destroy;

	FString Error;
	const bool bRegistered = FFlecsComponentReplicationRegistry::Get(InWorld).Register(MoveTemp(Descriptor), Error);

	if (OutError)
	{
		*OutError = MoveTemp(Error);
	}

	return bRegistered;
}

void UE::Flecs::Replication::MarkComponentReplicated(const FFlecsComponentHandle& InComponent)
{
	InComponent.GetFlecsWorldChecked()->RegisterComponentType<FFlecsReplicatedTrait>();

	InComponent.Add<FFlecsReplicatedTrait>();
}
