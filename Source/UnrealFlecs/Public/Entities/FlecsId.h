// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include <bit>

#include "flecs.h"

#include "CoreMinimal.h"

#include "SolidMacros/Macros.h"
#include "Standard/Hashing.h"
#include "Types/SolidNotNull.h"

#include "FlecsId.generated.h"

class UFlecsWorldInterfaceObject;

/**
 * @brief A Flecs Id equivalent to flecs::entity_t / flecs::id_t, has the same memory layout as uint64/flecs::id_t/flecs::entity_t
 */
USTRUCT(BlueprintType, meta = (
    DisableSplitPin,
    HasNativeMake = "/Script/UnrealFlecs.FlecsIdBlueprintFunctionLibrary.MakeFlecsId",
    HasNativeBreak = "/Script/UnrealFlecs.FlecsIdBlueprintFunctionLibrary.BreakFlecsId"))
struct UNREALFLECS_API FFlecsId
{
    GENERATED_BODY()

    NO_DISCARD FORCEINLINE friend uint32 GetTypeHash(const FFlecsId& InId)
    {
        return GetTypeHash(InId.GetId());
    }
    
    NO_DISCARD FORCEINLINE constexpr static FFlecsId Make(const uint32 InIndex, const uint32 InGeneration = 0)
    {
        return FFlecsId(InIndex, InGeneration);
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
        : Id(std::bit_cast<int64>(InId))
    {
    }

    FORCEINLINE FFlecsId(const flecs::entity InEntity)
        : FFlecsId(InEntity.id())
    {
    }
    
    FORCEINLINE explicit constexpr FFlecsId(const uint32 InIndex, const uint32 InGeneration = 0)
        : FFlecsId(ecs_entity_t_comb(InIndex, InGeneration))
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
        return GetId() < Other.GetId();
    }

    NO_DISCARD FORCEINLINE bool operator>(const FFlecsId& Other) const
    {
        return GetId() > Other.GetId();
    }

    NO_DISCARD FORCEINLINE bool operator<=(const FFlecsId& Other) const
    {
        return GetId() <= Other.GetId();
    }

    NO_DISCARD FORCEINLINE bool operator>=(const FFlecsId& Other) const
    {
        return GetId() >= Other.GetId();
    }

    NO_DISCARD FORCEINLINE bool IsPair() const
    {
        return ECS_IS_PAIR(GetId());
    }
    
    NO_DISCARD FORCEINLINE bool HasRelation(const FFlecsId InRelation) const
    {
        solid_checkf(IsPair(), TEXT("Id is not a pair."));
        return ECS_HAS_RELATION(GetId(), InRelation);
    }

    NO_DISCARD FORCEINLINE bool HasTarget(const FFlecsId InTarget) const
    {
        solid_checkf(IsPair(), TEXT("Id is not a pair."));
        return ECS_HAS_RELATION(GetId(), InTarget);
    }

    NO_DISCARD FORCEINLINE FFlecsId GetFirst() const
    {
        solid_checkf(IsPair(), TEXT("Id is not a pair."));
        return FFlecsId(ECS_PAIR_FIRST(GetId()));
    }

    NO_DISCARD FORCEINLINE FFlecsId GetSecond() const
    {
        solid_checkf(IsPair(), TEXT("Id is not a pair."));
        return FFlecsId(ECS_PAIR_SECOND(GetId()));
    }

    NO_DISCARD FORCEINLINE FFlecsId GetRelation() const
    {
        return GetFirst();
    }

    NO_DISCARD FORCEINLINE FFlecsId GetTarget() const
    {
        return GetSecond();
    }

    NO_DISCARD FORCEINLINE constexpr flecs::id_t GetId() const
    {
        return std::bit_cast<flecs::id_t>(Id);
    }

    NO_DISCARD FORCEINLINE TTuple<FFlecsId, FFlecsId> GetPairElements() const
    {
        solid_checkf(IsPair(), TEXT("Id is not a pair."));
        return TTuple<FFlecsId, FFlecsId>(GetFirst(), GetSecond());
    }
    
    NO_DISCARD FORCEINLINE const ecs_type_info_t* GetTypeInfo(const flecs::world& World) const
    {
        return ecs_get_type_info(World.c_ptr(), GetId());
    }
    
    NO_DISCARD FORCEINLINE const ecs_type_info_t* GetTypeInfo(const TSolidNotNull<const UFlecsWorldInterfaceObject*> World) const;

    FORCEINLINE operator flecs::id_t() const
    {
        return GetId();
    }

    NO_DISCARD FORCEINLINE uint32 GetIndex() const
    {
        solid_checkf(!IsPair(), TEXT("Id must not be a pair."));
        return GetId() & ECS_ENTITY_MASK;
    }

    NO_DISCARD FORCEINLINE uint32 GetGeneration() const
    {
        solid_checkf(!IsPair(), TEXT("Id must not be a pair."));
        return flecs::get_generation(GetId());
    }

    template <typename THandle>
    NO_DISCARD FORCEINLINE THandle ToHandle(const flecs::world& World) const
    {
        solid_checkf(IsValid(), TEXT("Id is not valid."));
        return World.get_alive(GetId());
    }

    NO_DISCARD FString ToString() const;

    bool ImportTextItem(const TCHAR*& Buffer, int32 PortFlags, UObject* Parent, FOutputDevice* ErrorText);

    bool ExportTextItem(FString& ValueStr, const FFlecsId& DefaultValue,
                        UObject* Parent, int32 PortFlags, UObject* ExportRootScope) const;

    UPROPERTY()
    int64 Id = 0;
    
}; // struct FFlecsId

static_assert(sizeof(FFlecsId) == sizeof(flecs::entity_t), "FFlecsId must have the same size as flecs::id_t");
static_assert(alignof(FFlecsId) == alignof(flecs::entity_t), "FFlecsId must have the same alignment as flecs::id_t");
static_assert(std::is_trivially_copyable_v<FFlecsId>, "FFlecsId must be trivially copyable.");

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
