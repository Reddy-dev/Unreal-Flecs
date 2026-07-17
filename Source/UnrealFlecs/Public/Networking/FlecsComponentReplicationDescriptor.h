// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Concepts/SolidConcepts.h"
#include "Entities/FlecsComponentHandle.h"
#include "Networking/FlecsReplicationQuantizers.h"
#include "Worlds/FlecsWorld.h"

#include "FlecsComponentReplicationDescriptor.generated.h"

class FArchive;

/**
 * Portable identity of a replicated component schema.
 *
 * The value is derived from the component's stable name and is exchanged in
 * layouts instead of a world-local Flecs ID. The layout's codec fingerprint
 * provides the serialization/quantization compatibility boundary.
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

	friend bool operator==(const FFlecsReplicationSchemaId&, const FFlecsReplicationSchemaId&) = default;
	
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
 * details. SchemaId, StableName, and CodecFingerprint form the portable
 * contract checked before a received layout can be applied.
 */
struct UNREALFLECS_API FFlecsComponentReplicationDescriptor
{
	FFlecsReplicationSchemaId SchemaId;
	FString StableName;
	FFlecsId LocalFlecsId;
	uint32 Size = 0;
	uint16 Alignment = 0;
	bool bIsTag = false;
	TObjectPtr<UScriptStruct> ScriptStruct = nullptr;
	FFlecsReplicationSerializeFunction Serialize = nullptr;
	FFlecsReplicationSerializeFunction QuantizeAndSerialize = nullptr;
	FFlecsReplicationSerializeFunction Deserialize = nullptr;
	FFlecsReplicationConstructFunction Construct = nullptr;
	FFlecsReplicationDestroyFunction Destroy = nullptr;
	FString CodecFingerprint = TEXT("None");
	float UpdateFrequencyHz = 0.0f;
	float ReplicationPriority = 1.0f;

	/** Validates the complete local descriptor before it enters the registry. */
	NO_DISCARD bool IsValid(FString* OutError = nullptr) const;
};

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
	using Quantizer = FFlecsNoReplicationQuantizer;

	static constexpr float UpdateFrequencyHz = 0.0f;
	static constexpr float ReplicationPriority = 1.0f;

	static NO_DISCARD FString StableSymbolName()
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
	bool Register(FFlecsComponentReplicationDescriptor Descriptor, FString& OutError);
	
	/** Finds a descriptor by a world-local Flecs ID. */
	NO_DISCARD const FFlecsComponentReplicationDescriptor* Find(FFlecsId LocalId) const;
	
	/** Finds a descriptor by its portable protocol schema ID. */
	NO_DISCARD const FFlecsComponentReplicationDescriptor* Find(FFlecsReplicationSchemaId SchemaId) const;
	
	NO_DISCARD const TMap<FFlecsId, FFlecsComponentReplicationDescriptor>& GetDescriptors() const
	{
		return ByLocalId;
	}
	
	NO_DISCARD FOnDescriptorRegistered& OnDescriptorRegistered()
	{
		return DescriptorRegisteredDelegate;
	}

	/** Rejects reflected types that contain unsupported raw object references. */
	static NO_DISCARD bool ValidateReflectedType(const TSolidNotNull<const UScriptStruct*> ScriptStruct, FString& OutError);

private:
	TMap<FFlecsId, FFlecsComponentReplicationDescriptor> ByLocalId;
	TMap<FFlecsReplicationSchemaId, FFlecsId> SchemaToLocalId;
	FOnDescriptorRegistered DescriptorRegisteredDelegate;
};

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

		if constexpr (requires { typename TFlecsReplicationTraits<T>::Quantizer; })
		{
			using FQuantizer = typename TFlecsReplicationTraits<T>::Quantizer;
			if constexpr (requires { FQuantizer::GetFingerprint(); })
			{
				Descriptor.CodecFingerprint = FQuantizer::GetFingerprint();
			}
			else if constexpr (requires { FQuantizer::Fingerprint; })
			{
				Descriptor.CodecFingerprint = FQuantizer::Fingerprint;
			}

			if constexpr (std::is_copy_constructible_v<T>
				&& requires(T& Value) { FQuantizer::Quantize(Value); })
			{
				Descriptor.QuantizeAndSerialize = [](FArchive& Archive, void* Value)
				{
					T Copy = *static_cast<T*>(Value);
					FQuantizer::Quantize(Copy);
					return static_cast<bool>(TFlecsReplicationTraits<T>::Serialize(Archive, Copy));
				};
			}
		}

		if (!Descriptor.QuantizeAndSerialize)
		{
			Descriptor.QuantizeAndSerialize = Descriptor.Serialize;
		}

		if constexpr (requires { TFlecsReplicationTraits<T>::UpdateFrequencyHz; })
		{
			Descriptor.UpdateFrequencyHz = FMath::Max(0.0f, TFlecsReplicationTraits<T>::UpdateFrequencyHz);
		}

		if constexpr (requires { TFlecsReplicationTraits<T>::ReplicationPriority; })
		{
			Descriptor.ReplicationPriority = FMath::Max(0.0f, TFlecsReplicationTraits<T>::ReplicationPriority);
		}

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
}
