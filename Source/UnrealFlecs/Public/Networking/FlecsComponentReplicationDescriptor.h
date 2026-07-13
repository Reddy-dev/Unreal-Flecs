// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Concepts/SolidConcepts.h"
#include "Entities/FlecsComponentHandle.h"
#include "Worlds/FlecsWorld.h"

#include "FlecsComponentReplicationDescriptor.generated.h"

class FArchive;

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsReplicationSchemaId
{
	GENERATED_BODY()

	FFlecsReplicationSchemaId() = default;
	explicit FFlecsReplicationSchemaId(const FGuid& InValue) : Value(InValue) {}

	static FFlecsReplicationSchemaId FromStableName(const FString& StableName);
	NO_DISCARD bool IsValid() const { return Value.IsValid(); }
	NO_DISCARD FString ToString() const { return Value.ToString(EGuidFormats::DigitsWithHyphensLower); }

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
};

template<>
struct TStructOpsTypeTraits<FFlecsReplicationSchemaId> : TStructOpsTypeTraitsBase2<FFlecsReplicationSchemaId>
{
	enum { WithIdenticalViaEquality = true };
};

using FFlecsReplicationSerializeFunction = bool(*)(FArchive&, void*);
using FFlecsReplicationConstructFunction = void(*)(void*);
using FFlecsReplicationDestroyFunction = void(*)(void*);

struct UNREALFLECS_API FFlecsComponentReplicationDescriptor
{
	FFlecsReplicationSchemaId SchemaId;
	FString StableName;
	uint32 SchemaVersion = 0;
	FFlecsId LocalFlecsId;
	uint32 Size = 0;
	uint16 Alignment = 0;
	bool bIsTag = false;
	TObjectPtr<UScriptStruct> ScriptStruct = nullptr;
	FFlecsReplicationSerializeFunction Serialize = nullptr;
	FFlecsReplicationSerializeFunction Deserialize = nullptr;
	FFlecsReplicationConstructFunction Construct = nullptr;
	FFlecsReplicationDestroyFunction Destroy = nullptr;

	NO_DISCARD bool IsValid(FString* OutError = nullptr) const;
};

/**
 * Replication customization point. Reflected USTRUCTs use their /Script path and
 * SerializeItem by default. Native types must specialize all three members.
 */
template <typename T>
struct TFlecsReplicationTraits
{
	static FString StableName()
	{
		if constexpr (Solid::IsScriptStruct<T>())
		{
			return TBaseStructure<T>::Get()->GetPathName();
		}
		else
		{
			return {};
		}
	}

	static constexpr uint32 SchemaVersion = Solid::IsScriptStruct<T>() ? 1u : 0u;

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
};

class UNREALFLECS_API FFlecsComponentReplicationRegistry
{
public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnDescriptorRegistered, const FFlecsComponentReplicationDescriptor&);

	static FFlecsComponentReplicationRegistry& Get(const UFlecsWorld* World);
	static void RemoveWorld(const UFlecsWorld* World);

	bool Register(FFlecsComponentReplicationDescriptor Descriptor, FString& OutError);
	NO_DISCARD const FFlecsComponentReplicationDescriptor* Find(FFlecsId LocalId) const;
	NO_DISCARD const FFlecsComponentReplicationDescriptor* Find(FFlecsReplicationSchemaId SchemaId) const;
	NO_DISCARD const TMap<FFlecsId, FFlecsComponentReplicationDescriptor>& GetDescriptors() const { return ByLocalId; }
	FOnDescriptorRegistered& OnDescriptorRegistered() { return DescriptorRegisteredDelegate; }

	static bool ValidateReflectedType(const UScriptStruct* ScriptStruct, FString& OutError);

private:
	TMap<FFlecsId, FFlecsComponentReplicationDescriptor> ByLocalId;
	TMap<FFlecsReplicationSchemaId, FFlecsId> SchemaToLocalId;
	FOnDescriptorRegistered DescriptorRegisteredDelegate;
};

namespace UE::Flecs::Replication
{
	template <typename T>
	bool RegisterComponent(const TSolidNotNull<const UFlecsWorld*> World, const FFlecsComponentHandle& Component,
		FString* OutError = nullptr)
	{
		FFlecsComponentReplicationDescriptor Descriptor;
		if constexpr (requires { TFlecsReplicationTraits<T>::StableName(); })
		{
			Descriptor.StableName = TFlecsReplicationTraits<T>::StableName();
		}
		Descriptor.SchemaId = FFlecsReplicationSchemaId::FromStableName(Descriptor.StableName);

		if constexpr (requires { TFlecsReplicationTraits<T>::SchemaVersion; })
		{
			Descriptor.SchemaVersion = TFlecsReplicationTraits<T>::SchemaVersion;
		}

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
}
