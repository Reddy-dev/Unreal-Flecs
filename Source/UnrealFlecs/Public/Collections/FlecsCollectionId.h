// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "SolidMacros/Macros.h"
#include "Standard/Hashing.h"

#include "FlecsCollectionId.generated.h"

/**
 * @brief Stable name-based identifier for a registered Unreal-Flecs Collection.
 *
 * Collection identifiers are used as keys by
 * UFlecsCollectionWorldSubsystem. They are local to the Flecs world that
 * registered the Collection; they do not represent a native Flecs entity id.
 */
USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsCollectionId
{
	GENERATED_BODY()
	
	/** Returns a hash for use in Unreal and standard hash containers. */
	NO_DISCARD FORCEINLINE friend uint32 GetTypeHash(const FFlecsCollectionId& InId)
	{
    	return GetTypeHash(InId.NameId);
    }

	/** Creates a Collection identifier from a name string. */
	static NO_DISCARD FORCEINLINE FFlecsCollectionId Make(const FString& InString)
	{
		return FFlecsCollectionId(InString);
	}

public:
	/** Creates an invalid, empty Collection identifier. */
	FORCEINLINE FFlecsCollectionId() = default;

	/** Creates a Collection identifier whose key is InNameId. */
	FORCEINLINE FFlecsCollectionId(const FString& InNameId)
		: NameId(InNameId)
	{
	}
	
	/** Compares Collection identifiers by their name key. */
	FORCEINLINE bool UEOpEquals(const FFlecsCollectionId& Other) const
	{
    	return NameId == Other.NameId;
    }

	/** Name used to look up the registered Collection. */
	UPROPERTY(EditAnywhere)
	FString NameId;
	
}; // struct FFlecsCollectionId

DEFINE_STD_HASH(FFlecsCollectionId);
