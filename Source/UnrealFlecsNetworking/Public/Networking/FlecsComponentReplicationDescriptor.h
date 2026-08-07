// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SolidMacros/Macros.h"
#include "Concepts/SolidConcepts.h"

#include "Entities/FlecsComponentHandle.h"
#include "Properties/FlecsReplicationComponentDefinition.h"
#include "Worlds/FlecsWorld.h"

#include "FlecsComponentReplicationDescriptor.generated.h"

class FArchive;

/**
 * Portable identity of a replicated component schema.
 *
 * The value is derived from the component's stable name and is exchanged in
 * layouts instead of a world-local Flecs ID. Client and server must use the
 * same protocol build.
 */
USTRUCT(BlueprintType)
struct UNREALFLECSNETWORKING_API FFlecsReplicationSchemaId
{
	GENERATED_BODY()

	FFlecsReplicationSchemaId() = default;
	explicit FFlecsReplicationSchemaId(const FGuid& InValue) 
		: Value(InValue)
	{
	}
	
	/** Creates a deterministic schema ID from a non-empty protocol stable name. */
	static NO_DISCARD FFlecsReplicationSchemaId FromStableName(const FString& StableName);
	
	NO_DISCARD FORCEINLINE bool IsValid() const
	{
		return Value.IsValid();
	}
	
	NO_DISCARD FORCEINLINE FString ToString() const
	{
		return Value.ToString(EGuidFormats::DigitsWithHyphensLower);
	}
	
	FORCEINLINE bool operator==(const FFlecsReplicationSchemaId& Other) const
	{
		return Value == Other.Value;
	}
	
	friend bool operator<(const FFlecsReplicationSchemaId& A, const FFlecsReplicationSchemaId& B)
	{
		if (A.Value.A != B.Value.A)
		{
			return A.Value.A < B.Value.A;
		}

		if (A.Value.B != B.Value.B)
		{
			return A.Value.B < B.Value.B;
		}

		if (A.Value.C != B.Value.C)
		{
			return A.Value.C < B.Value.C;
		}
		
		
		return A.Value.D < B.Value.D;
	}

	FORCEINLINE friend uint32 GetTypeHash(const FFlecsReplicationSchemaId& Id)
	{
		return GetTypeHash(Id.Value);
	}

	UPROPERTY()
	FGuid Value;
	
}; // struct FFlecsReplicationSchemaId

template<>
struct TStructOpsTypeTraits<FFlecsReplicationSchemaId> : TStructOpsTypeTraitsBase2<FFlecsReplicationSchemaId>
{
	enum
	{
		WithIdenticalViaEquality = true
	};
}; // struct TStructOpsTypeTraits<FFlecsReplicationSchemaId>

/**
 * Per-world description of one component that may appear in replication.
 *
 * LocalFlecsId and the lifetime/serialization callbacks are local runtime
 * details. SchemaId and StableName form the portable identity checked before
 * a received layout can be applied.
 */
struct UNREALFLECSNETWORKING_API FFlecsComponentReplicationDescriptor
{
	FFlecsReplicationSchemaId SchemaId;
	FString StableName;
	FFlecsId LocalFlecsId;
	
	uint32 Size = 0;
	uint16 Alignment = 0;
	
	bool bIsTag = false;
	
	TObjectPtr<UScriptStruct> ScriptStruct = nullptr;
	
	FFlecsReplicationSerializeFunction Serialize = nullptr;
	FFlecsReplicationSerializeFunction Deserialize = nullptr;
	FFlecsReplicationConstructFunction Construct = nullptr;
	FFlecsReplicationDestroyFunction Destroy = nullptr;

	/** Validates the complete local descriptor before it enters the registry. */
	NO_DISCARD bool IsValid(OUT FString* OutError = nullptr) const;
	
	NO_DISCARD FORCEINLINE FFlecsReplicationSchemaId GetSchemaId() const
	{
		return SchemaId;
	}
	
	NO_DISCARD FORCEINLINE FString GetStableName() const
	{
		return StableName;
	}
	
	NO_DISCARD FORCEINLINE FFlecsId GetLocalFlecsId() const
	{
		return LocalFlecsId;
	}
	
	NO_DISCARD FORCEINLINE uint32 GetSize() const
	{
		return Size;
	}
	
	NO_DISCARD FORCEINLINE uint16 GetAlignment() const
	{
		return Alignment;
	}
	
	NO_DISCARD FORCEINLINE bool IsTag() const
	{
		return bIsTag;
	}
	
	NO_DISCARD FORCEINLINE bool IsScriptStruct() const
	{
		return ScriptStruct != nullptr;
	}
	
	NO_DISCARD FORCEINLINE UScriptStruct* GetScriptStruct() const
	{
		return ScriptStruct;
	}
	
	NO_DISCARD FORCEINLINE FFlecsReplicationSerializeFunction GetSerializeFunction() const
	{
		return Serialize;
	}
	
	NO_DISCARD FORCEINLINE FFlecsReplicationSerializeFunction GetDeserializeFunction() const
	{
		return Deserialize;
	}
	
	NO_DISCARD FORCEINLINE FFlecsReplicationConstructFunction GetConstructFunction() const
	{
		return Construct;
	}
	
	NO_DISCARD FORCEINLINE FFlecsReplicationDestroyFunction GetDestroyFunction() const
	{
		return Destroy;
	}
	
	NO_DISCARD FORCEINLINE bool IsStorageEligible() const
	{
		return Size > 0 && Alignment > 0 && !bIsTag;
	}
	
}; // struct FFlecsComponentReplicationDescriptor

/**
 * Per-UFlecsWorld lookup table between portable schemas and local Flecs IDs.
 *
 * Component registration populates this registry when a type has
 * TFlecsComponentTraits<T>::Replicate enabled. The network subsystem listens
 * for new descriptors so components registered after world initialization are
 * also observed for dirty state.
 */
class UNREALFLECSNETWORKING_API FFlecsComponentReplicationRegistry
{
public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnDescriptorRegistered, const FFlecsComponentReplicationDescriptor&);
		
	static NO_DISCARD FFlecsComponentReplicationRegistry& Get(const TSolidNotNull<const UFlecsWorld*> World);
	
	/** Removes the registry during Flecs world teardown. */
	static void RemoveWorld(const UFlecsWorld* World);

	/** Adds a valid descriptor, rejecting schema IDs already owned by another local ID. */
	bool Register(const FFlecsComponentReplicationDescriptor& Descriptor, OUT FString& OutError);
	
	/** Finds a descriptor by a world-local Flecs ID. */
	NO_DISCARD const FFlecsComponentReplicationDescriptor* Find(const FFlecsId LocalId) const;
	
	/** Finds a descriptor by its portable protocol schema ID. */
	NO_DISCARD const FFlecsComponentReplicationDescriptor* Find(const FFlecsReplicationSchemaId& SchemaId) const;
	
	NO_DISCARD FORCEINLINE const TMap<FFlecsId, FFlecsComponentReplicationDescriptor>& GetDescriptors() const
	{
		return ByLocalId;
	}
	
	NO_DISCARD FORCEINLINE FOnDescriptorRegistered& OnDescriptorRegistered()
	{
		return DescriptorRegisteredDelegate;
	}

	/** Rejects reflected types that contain unsupported raw object references. */
	static NO_DISCARD bool ValidateReflectedType(const TSolidNotNull<const UScriptStruct*> ScriptStruct, FString& OutError);
	
	// @TODO: Poor name.
	/**
	 * if opted in for replication, this checks if the entity is eligible for replication 
	 * although stable paths and symbols can be replicated, that does not mean that every single instance of once is replicated.
	 **/
	static NO_DISCARD bool IsEntityReplicationEligible(const TSolidNotNull<const UFlecsWorld*> World, const FFlecsId Id);

private:
	TMap<FFlecsId, FFlecsComponentReplicationDescriptor> ByLocalId;
	TMap<FFlecsReplicationSchemaId, FFlecsId> SchemaToLocalId;
	
	FOnDescriptorRegistered DescriptorRegisteredDelegate;
	
}; // class FFlecsComponentReplicationRegistry

namespace UE::Flecs::Replication
{
	UNREALFLECSNETWORKING_API bool RegisterComponentDefinition(
		const TSolidNotNull<const UFlecsWorld*> InWorld,
		const FFlecsReplicationComponentDefinition& InDefinition,
		OUT FString* OutError = nullptr);

	UNREALFLECSNETWORKING_API void MarkComponentReplicated(const FFlecsComponentHandle& InComponent);

	/** Registers a replicated component through the networking module. */
	template <typename T>
	bool RegisterComponent(
		const TSolidNotNull<const UFlecsWorld*> InWorld,
		const FFlecsComponentHandle& InComponent,
		OUT FString* OutError = nullptr)
	{
		const FFlecsReplicationComponentDefinition Definition = UE::Flecs::Replication::MakeComponentDefinition<T>(InComponent);
		return RegisterComponentDefinition(InWorld, Definition, OutError);
	}
	
} // namespace UE::Flecs::Replication
