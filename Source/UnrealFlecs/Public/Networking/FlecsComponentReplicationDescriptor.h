// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SolidMacros/Macros.h"
#include "Concepts/SolidConcepts.h"

#include "Entities/FlecsComponentHandle.h"
#include "Worlds/FlecsWorld.h"

#include "FlecsComponentReplicationDescriptor.generated.h"

class FArchive;

/**
 * Portable identity of a replicated component schema.
 *
 * The value is derived from the component's stable name and is exchanged in
 * layouts instead of a world-local Flecs ID. It identifies a schema, not a
 * serializer version; use the descriptor's SchemaVersion for compatibility.
 */
USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsReplicationSchemaId
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
};

using FFlecsReplicationSerializeFunction = bool(*)(FArchive&, void*);
using FFlecsReplicationConstructFunction = void(*)(void*);
using FFlecsReplicationDestroyFunction = void(*)(void*);

/**
 * Per-world description of one component that may appear in replication.
 *
 * LocalFlecsId and the lifetime/serialization callbacks are local runtime
 * details. SchemaId, StableName, and SchemaVersion form the portable contract
 * checked before a received layout can be applied.
 */
struct UNREALFLECS_API FFlecsComponentReplicationDescriptor
{
	FFlecsReplicationSchemaId SchemaId;
	FString StableName;
	FFlecsId LocalFlecsId;
	
	uint32 Size = 0;
	uint16 Alignment = 0;
	
	// @TODO: Deprecated Maybe?
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
 * Replication customization point for a component type.
 *
 * Reflected USTRUCTs use their `/Script/...` path, schema version 1, and
 * SerializeItem by default. Native types must provide a stable name, a
 * nonzero schema version, and Serialize. Serialize is used in both archive
 * directions and must return false when the archive cannot be processed.
 */
template <typename T>
struct TFlecsReplicationTraits
{
	static FString StableSymbolName()
	{
		if constexpr (Solid::IsScriptStruct<T>())
		{
			return TBaseStructure<T>::Get()->GetStructCPPName();
		}
		else
		{
			return FString(nameof(T).data());
		}
	}

	static bool Serialize(FArchive& Archive, T& Value)
	{
		if constexpr (Solid::IsScriptStruct<T>())
		{
			TBaseStructure<T>::Get()->SerializeItem(Archive, &Value, nullptr);
			return !Archive.IsError();
		}
		else
		{
			return false;
		}
	}
	
}; // struct TFlecsReplicationTraits<T>

/**
 * Per-UFlecsWorld lookup table between portable schemas and local Flecs IDs.
 *
 * Component registration populates this registry when a type has
 * TFlecsComponentTraits<T>::Replicate enabled. The network subsystem listens
 * for new descriptors so components registered after world initialization are
 * also observed for dirty state.
 */
class UNREALFLECS_API FFlecsComponentReplicationRegistry
{
public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnDescriptorRegistered, const FFlecsComponentReplicationDescriptor&);

	/** Returns the registry owned by the supplied Flecs world. */
	static NO_DISCARD FFlecsComponentReplicationRegistry& Get(const TSolidNotNull<const UFlecsWorld*> World);
	
	/** Removes the registry during Flecs world teardown. */
	static void RemoveWorld(const UFlecsWorld* World);

	/** Adds a valid descriptor, rejecting schema IDs already owned by another local ID. */
	bool Register(FFlecsComponentReplicationDescriptor Descriptor, OUT FString& OutError);
	
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
	/**
	 * Registers T as a replicated component in World.
	 *
	 * This is called by normal component registration for a type whose
	 * component traits enable Replicate. Callers normally configure traits
	 * instead of invoking it directly.
	 */
	template <typename T>
	bool RegisterComponent(const TSolidNotNull<const UFlecsWorld*> World, const FFlecsComponentHandle& Component,
		FString* OutError = nullptr)
	{
		FFlecsComponentReplicationDescriptor Descriptor;
		if constexpr (requires { TFlecsReplicationTraits<T>::StableSymbolName(); })
		{
			Descriptor.StableName = TFlecsReplicationTraits<T>::StableSymbolName();
		}
		else if constexpr (requires { TFlecsReplicationTraits<T>::StableName(); })
		{
			Descriptor.StableName = TFlecsReplicationTraits<T>::StableName();
		}
		
		Descriptor.SchemaId = FFlecsReplicationSchemaId::FromStableName(Descriptor.StableName);
		
		Descriptor.LocalFlecsId = Component.GetFlecsId();
		Descriptor.Size = sizeof(T);
		Descriptor.Alignment = alignof(T);
		Descriptor.bIsTag = std::is_empty_v<T>;

		if constexpr (Solid::IsScriptStruct<T>())
		{
			Descriptor.ScriptStruct = TBaseStructure<T>::Get();
		}

		if constexpr (requires(FArchive& Archive, T& Value)
			{ TFlecsReplicationTraits<T>::Serialize(Archive, Value); })
		{
			Descriptor.Serialize = [](FArchive& Archive, void* Value)
			{
				return static_cast<bool>(TFlecsReplicationTraits<T>::Serialize(Archive, *static_cast<T*>(Value)));
			};
		}

		Descriptor.Deserialize = Descriptor.Serialize;

		if constexpr (std::is_default_constructible_v<T>)
		{
			Descriptor.Construct = [](void* Value)
			{
				if constexpr (Solid::IsScriptStruct<T>())
				{
					TBaseStructure<T>::Get()->InitializeStruct(Value);
				}
				else
				{
					new (Value) T();
				}
			};
		}

		if constexpr (std::is_destructible_v<T>)
		{
			Descriptor.Destroy = [](void* Value)
			{
				if constexpr (Solid::IsScriptStruct<T>())
				{
					TBaseStructure<T>::Get()->DestroyStruct(Value);
				}
				else
				{
					static_cast<T*>(Value)->~T();
				}
			};
		}

		FString Error;
		const bool bRegistered = FFlecsComponentReplicationRegistry::Get(World).Register(MoveTemp(Descriptor), Error);

		if (OutError)
		{
			*OutError = Error;
		}

		return bRegistered;
	}
	
} // namespace UE::Flecs::Replication
