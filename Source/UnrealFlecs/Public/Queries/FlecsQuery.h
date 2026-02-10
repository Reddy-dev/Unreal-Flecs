// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "flecs.h"

#include "CoreMinimal.h"

#include "SolidMacros/Macros.h"

#include "FlecsQueryBase.h"

#include "FlecsQuery.generated.h"

struct FFlecsQueryBuilder;
struct FFlecsQuery;

template <typename ...TArgs>
struct TTypedFlecsQuery;

class UFlecsWorld;

USTRUCT(BlueprintType)
struct FFlecsQuery : public FFlecsQueryBase
    #if CPP
        , public flecs::iterable<>
    #endif // CPP
{
    GENERATED_BODY()
    
private:
    using Fields = typename flecs::_::field_ptrs<>::array;

public:
    using FFlecsQueryBase::FFlecsQueryBase;
    
    FORCEINLINE FFlecsQuery()
        : FFlecsQueryBase()
    {
    }

    /*FORCEINLINE FFlecsQuery(const FFlecsQuery& Other)
        : FFlecsQueryBase(Other)
    {
    }*/
    
    FORCEINLINE FFlecsQuery(FFlecsQuery&& Other) noexcept
        : FFlecsQueryBase(FLECS_FWD(Other))
    {
    }
    
    FORCEINLINE FFlecsQuery& operator=(const FFlecsQuery&& Other)
    {
        FFlecsQueryBase::operator=(FLECS_FWD(Other));
        return *this;
    }
    
private:
    virtual ecs_iter_t get_iter(flecs::world_t *world) const override 
    {
        ecs_assert(GetCPtr() != nullptr, ECS_INVALID_PARAMETER, 
            "cannot iterate invalid query");
        if (!world) {
            world = GetCPtr()->world;
        }
        return ecs_query_iter(world, GetCPtr());
    }

    virtual ecs_iter_next_action_t next_action() const override 
    {
        return ecs_query_next;
    }
    
}; // struct FFlecsQuery

template <>
struct TStructOpsTypeTraits<FFlecsQuery> : public TStructOpsTypeTraitsBase2<FFlecsQuery>
{
    enum
    {
        WithCopy = false,
    };
};

template <typename ...TArgs>
struct TTypedFlecsQuery : public FFlecsQueryBase, public flecs::iterable<TArgs...>
{
private:
    using Fields = flecs::_::field_ptrs<TArgs...>::array;
    
public:
    using FFlecsQueryBase::FFlecsQueryBase;
    
    FORCEINLINE TTypedFlecsQuery() 
        : FFlecsQueryBase()
    {
    }
    
    FORCEINLINE TTypedFlecsQuery(const TTypedFlecsQuery& Other) 
        : FFlecsQueryBase(Other)
    {
    }
    
    FORCEINLINE TTypedFlecsQuery(TTypedFlecsQuery&& Other) noexcept 
        : FFlecsQueryBase(FLECS_FWD(Other))
    {
    }
    
    FORCEINLINE TTypedFlecsQuery& operator=(const TTypedFlecsQuery&& Other) 
    {
        FFlecsQueryBase::operator=(FLECS_FWD(Other));
        return *this;
    }
    
    FORCEINLINE TTypedFlecsQuery(const FFlecsQuery& Other) 
        : FFlecsQueryBase(Other)
    {
    }
    
private:
    virtual ecs_iter_t get_iter(flecs::world_t *world) const override 
    {
        ecs_assert(GetCPtr() != nullptr, ECS_INVALID_PARAMETER, 
            "cannot iterate invalid query");
        
        if (!world) 
        {
            world = GetCPtr()->world;
        }
        
        return ecs_query_iter(world, GetCPtr());
    }

    virtual ecs_iter_next_action_t next_action() const override 
    {
        return ecs_query_next;
    }
    
    
}; // struct TTypedFlecsQuery


