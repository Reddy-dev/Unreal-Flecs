﻿// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "flecs.h"

#include "CoreMinimal.h"
#include "SolidMacros/Macros.h"
#include "Standard/Hashing.h"
#include "FlecsId.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsId
{
    GENERATED_BODY()

    NO_DISCARD FORCEINLINE friend uint32 GetTypeHash(const FFlecsId& InId)
    {
        return GetTypeHash(InId.Id);
    }

    /**
     * @brief Make a new Pair Id from the given First and Second Elements
     * @param InFirst First Element of the pair
     * @param InSecond Second Element of the pair
     * @return The combined pair Id
     */
    NO_DISCARD FORCEINLINE constexpr static FFlecsId MakePair(const FFlecsId InFirst, const FFlecsId InSecond)
    {
        return FFlecsId(ecs_pair(InFirst, InSecond));
    }

    /**
     * @return A Null Id without a World Context
     */
    NO_DISCARD FORCEINLINE constexpr static FFlecsId Null()
    {
        return FFlecsId(flecs::entity::null().id());
    }

public:
    FORCEINLINE FFlecsId() = default;

    FORCEINLINE constexpr FFlecsId(const flecs::id_t InId)
        : Id(InId)
    {
    }

    FORCEINLINE FFlecsId(const flecs::entity InEntity)
        : Id(InEntity.id())
    {
    }

    NO_DISCARD FORCEINLINE bool IsValid() const
    {
        return Id != 0;
    }
    
    NO_DISCARD FORCEINLINE bool operator==(const FFlecsId& Other) const
    {
        return Id == Other.Id;
    }

    NO_DISCARD FORCEINLINE bool operator!=(const FFlecsId& Other) const
    {
        return Id != Other.Id;
    }
    
    NO_DISCARD FORCEINLINE bool operator<(const FFlecsId& Other) const
    {
        return Id < Other.Id;
    }

    NO_DISCARD FORCEINLINE bool operator>(const FFlecsId& Other) const
    {
        return Id > Other.Id;
    }

    NO_DISCARD FORCEINLINE bool operator<=(const FFlecsId& Other) const
    {
        return Id <= Other.Id;
    }

    NO_DISCARD FORCEINLINE bool operator>=(const FFlecsId& Other) const
    {
        return Id >= Other.Id;
    }

    NO_DISCARD FORCEINLINE bool IsPair() const
    {
        return ECS_IS_PAIR(Id);
    }
    
    NO_DISCARD FORCEINLINE bool HasRelation(const FFlecsId InRelation) const
    {
        solid_checkf(IsPair(), TEXT("Id is not a pair."));
        return ECS_HAS_RELATION(Id, InRelation);
    }

    NO_DISCARD FORCEINLINE bool HasTarget(const FFlecsId InTarget) const
    {
        solid_checkf(IsPair(), TEXT("Id is not a pair."));
        return ECS_HAS_RELATION(Id, InTarget);
    }

    NO_DISCARD FORCEINLINE FFlecsId GetFirst() const
    {
        solid_checkf(IsPair(), TEXT("Id is not a pair."));
        return FFlecsId(ECS_PAIR_FIRST(Id));
    }

    NO_DISCARD FORCEINLINE FFlecsId GetSecond() const
    {
        solid_checkf(IsPair(), TEXT("Id is not a pair."));
        return FFlecsId(ECS_PAIR_SECOND(Id));
    }

    NO_DISCARD FORCEINLINE FFlecsId GetRelation() const
    {
        return GetFirst();
    }

    NO_DISCARD FORCEINLINE FFlecsId GetTarget() const
    {
        return GetSecond();
    }

    NO_DISCARD FORCEINLINE flecs::id_t GetId() const
    {
        return Id;
    }

    FORCEINLINE operator flecs::id_t() const
    {
        return GetId();
    }

    NO_DISCARD FORCEINLINE uint32 GetIndex() const
    {
        solid_checkf(!IsPair(), TEXT("Id must not be a pair."));
        return Id & ECS_ENTITY_MASK;
    }

    NO_DISCARD FORCEINLINE uint32 GetGeneration() const
    {
        solid_checkf(!IsPair(), TEXT("Id must not be a pair."));
        return flecs::get_generation(Id);
    }

    FORCEINLINE FString ToString() const
    {
        return FString::Printf(TEXT("Index: %d, Generation: %d"), GetIndex(), GetGeneration());
    }

    FORCEINLINE bool ImportTextItem(const TCHAR*& Buffer, int32 PortFlags, UObject* Parent, FOutputDevice* ErrorText)
    {
        uint64 TempId = 0;
        
        if LIKELY_IF(FParse::Value(Buffer, TEXT("FlecsId="), TempId))
        {
            Id = TempId;
            return true;
        }

        return false;
    }

    FORCEINLINE bool ExportTextItem(FString& ValueStr, const FFlecsId& DefaultValue,
        UObject* Parent, int32 PortFlags, UObject* ExportRootScope) const
    {
        ValueStr = FString::Printf(TEXT("FlecsId=%llu"), Id);
        return true;
    }

    
    UPROPERTY()
    uint64 Id = 0;
    
}; // struct FFlecsId

template<>
struct TStructOpsTypeTraits<FFlecsId> : public TStructOpsTypeTraitsBase2<FFlecsId>
{
    enum
    {
        WithCopy = true,
        WithIdenticalViaEquality = true,
        WithImportTextItem = true,
        WithExportTextItem = true,
    }; // enum
    
}; // struct TStructOpsTypeTraits<FFlecsId>

DEFINE_STD_HASH(FFlecsId);
