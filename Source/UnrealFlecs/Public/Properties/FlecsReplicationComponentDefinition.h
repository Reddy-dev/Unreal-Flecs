// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include <new>
#include <type_traits>

#include "CoreMinimal.h"

#include "Concepts/SolidConcepts.h"
#include "Entities/FlecsComponentHandle.h"
#include "Serialization/Archive.h"

class FArchive;

/** Type-erased replication metadata produced by core component registration. */
using FFlecsReplicationSerializeFunction = bool(*)(FArchive&, void*);
using FFlecsReplicationConstructFunction = void(*)(void*);
using FFlecsReplicationDestroyFunction = void(*)(void*);

struct UNREALFLECS_API FFlecsReplicationComponentDefinition
{
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
}; // struct FFlecsReplicationComponentDefinition

/** Serialization customization point used when a component opts into replication. */
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

namespace UE::Flecs::Replication
{
	template <typename T>
	NO_DISCARD FFlecsReplicationComponentDefinition MakeComponentDefinition(
		const FFlecsComponentHandle& InComponent)
	{
		FFlecsReplicationComponentDefinition Definition;
		if constexpr (requires { TFlecsReplicationTraits<T>::StableSymbolName(); })
		{
			Definition.StableName = TFlecsReplicationTraits<T>::StableSymbolName();
		}
		else if constexpr (requires { TFlecsReplicationTraits<T>::StableName(); })
		{
			Definition.StableName = TFlecsReplicationTraits<T>::StableName();
		}

		Definition.LocalFlecsId = InComponent.GetFlecsId();
		Definition.Size = sizeof(T);
		Definition.Alignment = alignof(T);
		Definition.bIsTag = std::is_empty_v<T>;

		if constexpr (Solid::IsScriptStruct<T>())
		{
			Definition.ScriptStruct = TBaseStructure<T>::Get();
		}

		if constexpr (requires(FArchive& Archive, T& Value)
			{ TFlecsReplicationTraits<T>::Serialize(Archive, Value); })
		{
			Definition.Serialize = [](FArchive& Archive, void* Value)
			{
				return static_cast<bool>(TFlecsReplicationTraits<T>::Serialize(
					Archive, *static_cast<T*>(Value)));
			};
		}

		Definition.Deserialize = Definition.Serialize;

		if constexpr (std::is_default_constructible_v<T>)
		{
			Definition.Construct = [](void* Value)
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
			Definition.Destroy = [](void* Value)
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

		return Definition;
	}
} // namespace UE::Flecs::Replication
