// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsObserverDefinition.h"

#include "Queries/FlecsQueryBuilderBase.h"

template <typename TInherited, typename THandleType, typename ...TComponents>
struct TFlecsObserverBuilderBase : public TFlecsQueryBuilderBase<TInherited>
{
	
public:
	FORCEINLINE FFlecsObserverDefinition& GetObserverDefinition() const
	{
		return this->GetSelf().GetObserverDefinition_Impl();
	}
	
	FORCEINLINE FFlecsQueryDefinition& GetQueryDefinition_Impl() const
	{
		return const_cast<FFlecsQueryDefinition&>(GetObserverDefinition().QueryDefinition);
	}
	
	FORCEINLINE TInherited& Event(const EFlecsObserverEvent InEventType)
	{
		GetObserverDefinition().Events.Add(FFlecsObserverEventInput::Make(InEventType));
		return this->GetSelf();
	}
	
	FORCEINLINE TInherited& Event(const FFlecsId InEventId)
	{
		GetObserverDefinition().Events.Add(FFlecsObserverEventInput::Make(InEventId));
		return this->GetSelf();
	}
	
	FORCEINLINE TInherited& Event(const FFlecsObserverEventInput& InEventInput)
	{
		GetObserverDefinition().Events.Add(InEventInput);
		return this->GetSelf();
	}
	
	FORCEINLINE TInherited& YieldExisting(const bool bInYieldExisting = true)
	{
		GetObserverDefinition().bYieldExisting = bInYieldExisting;
		return this->GetSelf();
	}
	
	FORCEINLINE TInherited& ObserverFlags(const uint32 InFlags)
	{
		GetObserverDefinition().Flags = InFlags;
		return this->GetSelf();
	}
	
	FORCEINLINE TInherited& ObserverFlags(const EFlecsObserverFlags InFlags)
	{
		GetObserverDefinition().Flags = static_cast<uint32>(InFlags);
		return this->GetSelf();
	}
	
	template <typename Func>
	THandleType run(Func&& func) {
		using Delegate = typename flecs::_::run_delegate<
			typename std::decay<Func>::type>;

		auto ctx = FLECS_NEW(Delegate)(FLECS_FWD(func));
		GetObserverDefinition().run = Delegate::run;
		GetObserverDefinition().run_ctx = ctx;
		GetObserverDefinition().run_ctx_free = flecs::_::free_obj<Delegate>;
		return this->GetSelf().CreateRunObserver();
	}

	template <typename Func, typename EachFunc>
	THandleType run(Func&& func, EachFunc&& each_func) {
		using Delegate = typename flecs::_::run_delegate<
			typename std::decay<Func>::type>;

		auto ctx = FLECS_NEW(Delegate)(FLECS_FWD(func));
		GetObserverDefinition().run = Delegate::run;
		GetObserverDefinition().run_ctx = ctx;
		GetObserverDefinition().run_ctx_free = flecs::_::free_obj<Delegate>;
		return each(FLECS_FWD(each_func));
	}

	template <typename Func>
	THandleType each(Func&& func) {
		using Delegate = typename flecs::_::each_delegate<
			typename std::decay<Func>::type, TComponents...>;
		auto ctx = FLECS_NEW(Delegate)(FLECS_FWD(func));
		GetObserverDefinition().callback = Delegate::run;
		GetObserverDefinition().callback_ctx = ctx;
		GetObserverDefinition().callback_ctx_free = flecs::_::free_obj<Delegate>;
		return this->GetSelf().CreateEachObserver();
	}

	template <typename Func>
	THandleType run_each(Func&& func) {
		using Delegate = typename flecs::_::each_delegate<
			typename std::decay<Func>::type, TComponents...>;
		auto ctx = FLECS_NEW(Delegate)(FLECS_FWD(func));
		GetObserverDefinition().run = Delegate::run_each;
		GetObserverDefinition().run_ctx = ctx;
		GetObserverDefinition().run_ctx_free = flecs::_::free_obj<Delegate>;
		return this->GetSelf().CreateRunEachObserver();
	}
	
}; // struct TFlecsObserverBuilderBase
