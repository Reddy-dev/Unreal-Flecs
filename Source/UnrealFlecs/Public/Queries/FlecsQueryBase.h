// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "flecs.h"

#include "CoreMinimal.h"

#include "Entities/FlecsEntityHandle.h"

#include "FlecsQueryBase.generated.h"

struct FFlecsQueryDefinition;
struct FFlecsQuery;

USTRUCT(BlueprintInternalUseOnly)
struct UNREALFLECS_API FFlecsQueryBase
{
	GENERATED_BODY()
	
public:
	FORCEINLINE FFlecsQueryBase() = default;
	
	virtual ~FFlecsQueryBase() = default;
	
	FORCEINLINE FFlecsQueryBase(const flecs::query_base& InQueryBase)
		: QueryBase(InQueryBase)
	{
	}
	
	FORCEINLINE FFlecsQueryBase(const FFlecsQueryBase& Other)
		: QueryBase(Other.QueryBase)
	{
	}
	
	FORCEINLINE FFlecsQueryBase& operator=(const FFlecsQueryBase& Other)
	{
		if (this != &Other)
		{
			QueryBase = Other.QueryBase;
		}
		
		return *this;
	}

	FORCEINLINE FFlecsQueryBase(FFlecsQueryBase&& Other) NOEXCEPT
		: QueryBase(MoveTemp(Other.QueryBase))
	{
	}
	
	FORCEINLINE FFlecsQueryBase& operator=(FFlecsQueryBase&& Other) NOEXCEPT
	{
		if (this != &Other)
		{
			QueryBase = MoveTemp(Other.QueryBase);
		}
		
		return *this;
	}
	
	FORCEINLINE operator flecs::query_base&()
	{
		return QueryBase;
	}
	
	FORCEINLINE operator const flecs::query_base&() const
	{
		return QueryBase;
	}
	
	NO_DISCARD FORCEINLINE flecs::query_base& GetQueryBase()
	{
		return QueryBase;
	}
	
	NO_DISCARD FORCEINLINE const flecs::query_base& GetQueryBase() const
	{
		return QueryBase;
	}
	
	NO_DISCARD FORCEINLINE bool IsValid() const
	{
		return static_cast<bool>(QueryBase);
	}
	
	FORCEINLINE operator bool() const
	{
		return IsValid();
	}
	
	NO_DISCARD FORCEINLINE FFlecsEntityHandle GetEntity() const
	{
		solid_checkf(IsValid(), TEXT("Query is not valid."));
		return QueryBase.entity();
	}
	
	NO_DISCARD FORCEINLINE bool HasChanged() const
	{
		return QueryBase.changed();
	}
	
	NO_DISCARD FORCEINLINE int32 GetTermCount() const
	{
		return QueryBase.term_count();
	}
	
	NO_DISCARD FORCEINLINE int32 GetFieldCount() const
	{
		return QueryBase.field_count();
	}
	
	NO_DISCARD FORCEINLINE int32 FindVar(const FString& InVarName) const
	{
		return QueryBase.find_var(StringCast<char>(*InVarName).Get());
	}
	
	NO_DISCARD FORCEINLINE FString ToString() const
	{
		return FString(UTF8_TO_TCHAR(QueryBase.str().c_str()));
	}
	
	NO_DISCARD FORCEINLINE FString GetQueryPlan() const
	{
		return FString(UTF8_TO_TCHAR(QueryBase.plan().c_str()));
	}
	
	NO_DISCARD FORCEINLINE flecs::term GetTerm(const int32 InIndex) const
	{
		solid_checkf(InIndex >= 0 && InIndex < GetTermCount(),
			TEXT("Index %d is out of bounds [0, %d)"), InIndex, GetTermCount());
		
		return QueryBase.term(static_cast<size_t>(InIndex));
	}
	
	NO_DISCARD FORCEINLINE const flecs::query_group_info_t* GetGroupInfo(const uint64 InGroupId) const
	{
		return QueryBase.group_info(InGroupId);
	}
	
	NO_DISCARD FORCEINLINE void* GetGroupContext(const uint64 InGroupId) const
	{
		return QueryBase.group_ctx(InGroupId);
	}
	
	template <typename TFunction>
	FORCEINLINE void EachTerm(const TFunction& InFunction)
	{
		QueryBase.each_term(InFunction);
	}
	
	FORCEINLINE bool Destroy()
	{
		if LIKELY_IF(IsValid())
		{
			if (GetEntity().IsValid())
			{
				QueryBase.destruct();
			}
			
			QueryBase.query_ = nullptr;
			return true;
		}
		
		return false;
	}
	
	NO_DISCARD FORCEINLINE const flecs::query_t* GetCPtr() const
	{
		return QueryBase ? QueryBase.c_ptr() : nullptr;
	}
	
	NO_DISCARD FORCEINLINE FFlecsEntityHandle GetEntityHandle() const
	{
		solid_checkf(IsValid(), TEXT("Query is not valid."));
		return QueryBase.entity();
	}
	
	operator FFlecsQuery() const;
	
#ifdef FLECS_JSON
	
	// @TODO: does this need to be non-const
	NO_DISCARD FORCEINLINE FString ToJson(flecs::iter_to_json_desc_t* Desc = nullptr)
	{
		const flecs::string JsonString = QueryBase.to_json(Desc);
		return FString(UTF8_TO_TCHAR(JsonString.c_str()));
	}
	
#endif // #ifdef FLECS_JSON
	
	flecs::query_base QueryBase;
	
}; // struct FFlecsQueryBase

template <>
struct TStructOpsTypeTraits<FFlecsQueryBase> : public TStructOpsTypeTraitsBase2<FFlecsQueryBase>
{
	enum
	{
		WithCopy = false,
	};
};