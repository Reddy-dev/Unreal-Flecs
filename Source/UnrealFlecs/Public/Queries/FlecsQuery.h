// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "flecs.h"

#include "CoreMinimal.h"

#include "SolidMacros/Macros.h"

#include "Entities/FlecsEntityHandle.h"
#include "FlecsQueryDefinition.h"

#include "FlecsQuery.generated.h"

struct FFlecsQueryBuilder;
class UFlecsWorld;

USTRUCT(BlueprintType)
struct FFlecsQuery
{
    GENERATED_BODY()

    NO_DISCARD FORCEINLINE friend uint32 GetTypeHash(const FFlecsQuery& InQuery)
    {
        return GetTypeHash(InQuery.Query.c_ptr());
    }

public:
    FORCEINLINE FFlecsQuery() = default;
    
    FORCEINLINE FFlecsQuery(const flecs::query<>& InQuery) : Query(InQuery) {}
    FORCEINLINE FFlecsQuery(const flecs::query<>* InQuery) : Query(*InQuery) {}

    FORCEINLINE FFlecsQuery(flecs::query_builder<>& InQueryBuilder)
    {
        Query = InQueryBuilder.build();
    }

    template<typename ...TArgs>
    FORCEINLINE FFlecsQuery(const flecs::world& InWorld, const char* InName)
    {
        Query = InWorld.query<TArgs...>(InName);
    }

    FFlecsQuery(const TSolidNotNull<const UFlecsWorld*> InFlecsWorld, 
                const FFlecsQueryDefinition& InDefinition,
                const FString& InName = "");

    explicit FFlecsQuery(const FFlecsQueryBuilder& InQueryBuilder);
    
    NO_DISCARD FORCEINLINE bool IsValid() const
    {
        return Query;
    }
    
    NO_DISCARD FORCEINLINE const flecs::query<>& Get() const
    {
        return Query;
    }
    
    NO_DISCARD FORCEINLINE flecs::query<>& Get()
    {
        return Query;
    }
    
    NO_DISCARD FORCEINLINE bool HasChanged() const
    {
        return Query.changed();
    }

    NO_DISCARD FORCEINLINE int32 GetCount() const
    {
        return Query.count();
    }

    NO_DISCARD FORCEINLINE int32 GetFieldCount() const
    {
        return Query.field_count();
    }
    
    NO_DISCARD FORCEINLINE int32 GetTermCount() const
    {
        return Query.term_count();
    }

    NO_DISCARD FORCEINLINE bool HasMatches() const
    {
        return Query.is_true();
    }
    
    NO_DISCARD FORCEINLINE int32 FindVar(const FString& InVarName) const
    {
        const char* CStr = TCHAR_TO_UTF8(*InVarName);
        solid_cassume(CStr != nullptr);
        
        return Query.find_var(CStr);
    }

    NO_DISCARD FORCEINLINE FString ToString() const
    {
        return StringCast<TCHAR>(ecs_query_str(Query)).Get();
    }
    
    FORCEINLINE void Destroy()
    {
        Query.destruct();
    }

    NO_DISCARD FORCEINLINE bool operator==(const FFlecsQuery& Other) const
    {
        return Query.c_ptr() == Other.Query.c_ptr();
    }

    NO_DISCARD FORCEINLINE bool operator!=(const FFlecsQuery& Other) const
    {
        return !(*this == Other);
    }
    
    NO_DISCARD FORCEINLINE operator bool() const
    {
        return Query;
    }
    
    flecs::query<> Query;
    
}; // struct FFlecsQuery

/*template <typename ...TArgs>
struct TFlecsQuery
{
}; // struct TFlecsQuery*/


