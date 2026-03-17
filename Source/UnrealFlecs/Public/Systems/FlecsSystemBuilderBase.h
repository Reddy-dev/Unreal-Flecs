// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FlecsPhasesType.h"

#include "SolidMacros/Macros.h"

#include "Queries/FlecsQueryBuilderBase.h"
#include "FlecsSystemDefinition.h"

template <typename TInherited, typename THandleType, typename ...TComponents>
struct TFlecsSystemBuilderBase : public TFlecsQueryBuilderBase<TInherited>
{
public:
	FORCEINLINE FFlecsSystemDefinition& GetSystemDefinition() const
	{
		return this->GetSelf().GetSystemDefinition_Impl();
	}
	
	FORCEINLINE FFlecsQueryDefinition& GetQueryDefinition_Impl() const
	{
		return const_cast<FFlecsQueryDefinition&>(GetSystemDefinition().QueryDefinition);
	}
	
	FORCEINLINE TInherited& Kind(const EFlecsPhaseType InKind)
	{
		GetSystemDefinition().PhaseInput.Type = EFlecsSystemPhaseInputType::FlecsPhase;
		GetSystemDefinition().PhaseInput.FlecsPhase = InKind;
		return this->GetSelf();
	}
	
	FORCEINLINE TInherited& Kind(const FFlecsId InKind)
	{
		GetSystemDefinition().PhaseInput.Type = EFlecsSystemPhaseInputType::Type;
		GetSystemDefinition().PhaseInput.PhaseInput.First.InitializeAs<FFlecsQueryGeneratorInputType_FlecsId>();
		GetSystemDefinition().PhaseInput.PhaseInput.First.GetMutable<FFlecsQueryGeneratorInputType_FlecsId>().FlecsId = InKind;
		return this->GetSelf();
	}
	
	FORCEINLINE TInherited& Kind(const TSolidNotNull<const UScriptStruct*> InKind)
	{
		GetSystemDefinition().PhaseInput.Type = EFlecsSystemPhaseInputType::Type;
		GetSystemDefinition().PhaseInput.PhaseInput.First.InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		GetSystemDefinition().PhaseInput.PhaseInput.First.GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = InKind;
		return this->GetSelf();
	}
	
	FORCEINLINE TInherited& Kind(const FSolidEnumSelector& InKind)
	{
		GetSystemDefinition().PhaseInput.Type = EFlecsSystemPhaseInputType::Type;
		GetSystemDefinition().PhaseInput.PhaseInput.bPair = true;
		
		GetSystemDefinition().PhaseInput.PhaseInput.First.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnum>();
		GetSystemDefinition().PhaseInput.PhaseInput.First.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnum>().ScriptEnum = InKind.Class;
		
		GetSystemDefinition().PhaseInput.PhaseInput.Second.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnumConstant>();
		GetSystemDefinition().PhaseInput.PhaseInput.Second.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnumConstant>().EnumValue = InKind;
		
		return this->GetSelf();
	}
	
	FORCEINLINE TInherited& Kind(const FString& InCppTypeSymbol)
	{
		GetSystemDefinition().PhaseInput.Type = EFlecsSystemPhaseInputType::Type;
		GetSystemDefinition().PhaseInput.PhaseInput.First.InitializeAs<FFlecsQueryGeneratorInputType_CPPType>();
		GetSystemDefinition().PhaseInput.PhaseInput.First.GetMutable<FFlecsQueryGeneratorInputType_CPPType>().SymbolString = InCppTypeSymbol;
		return this->GetSelf();
	}
	
	template <typename T>
	FORCEINLINE TInherited& Kind()
	{
		const std::string_view TypeName = nameof(T);
		Kind(FString(TypeName.data()));
		
		return this->GetSelf();
	}
	
	FORCEINLINE TInherited& MultiThreaded(const bool bInMultiThreaded = true)
	{
		GetSystemDefinition().bMultiThreaded = bInMultiThreaded;
		return this->GetSelf();
	}
	
	FORCEINLINE TInherited& Immediate(const bool bInImmediate = true)
	{
		GetSystemDefinition().bImmediate = bInImmediate;
		return this->GetSelf();
	}
	
	FORCEINLINE TInherited& Interval(const double InInterval)
	{
		GetSystemDefinition().Interval = InInterval;
		return this->GetSelf();
	}
	
	FORCEINLINE TInherited& TickSource(const TSubclassOf<UFlecsSystemObject> InTickSource)
	{
		GetSystemDefinition().TickSourceInput.TypeInput = EFlecsSystemTickSourceInput::SystemClass;
		GetSystemDefinition().TickSourceInput.SystemClassInput = InTickSource;
		return this->GetSelf();
	}
	
	template <Solid::TStaticClassConcept T>
	requires (std::derived_from<T, UFlecsSystemObject>)
	FORCEINLINE TInherited& TickSource()
	{
		TickSource(T::StaticClass());
		
		return this->GetSelf();
	}
	
	FORCEINLINE TInherited& TickSource(const FFlecsId InTypeId)
	{
		GetSystemDefinition().TickSourceInput.TypeInput = EFlecsSystemTickSourceInput::Type;
		GetSystemDefinition().TickSourceInput.TypeInput.First.InitializeAs<FFlecsQueryGeneratorInputType_FlecsId>();
		GetSystemDefinition().TickSourceInput.TypeInput.First.GetMutable<FFlecsQueryGeneratorInputType_FlecsId>().FlecsId = InTypeId;
		return this->GetSelf();
	}
	
	FORCEINLINE TInherited& TickSource(const TSolidNotNull<const UScriptStruct*> InStruct)
	{
		GetSystemDefinition().TickSourceInput.TypeInput = EFlecsSystemTickSourceInput::Type;
		GetSystemDefinition().TickSourceInput.TypeInput.First.InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		GetSystemDefinition().TickSourceInput.TypeInput.First.GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = InStruct;
		return this->GetSelf();
	}
	
	FORCEINLINE TInherited& TickSource(const FString& InCppTypeSymbol)
	{
		GetSystemDefinition().TickSourceInput.TypeInput = EFlecsSystemTickSourceInput::Type;
		GetSystemDefinition().TickSourceInput.TypeInput.First.InitializeAs<FFlecsQueryGeneratorInputType_CPPType>();
		GetSystemDefinition().TickSourceInput.TypeInput.First.GetMutable<FFlecsQueryGeneratorInputType_CPPType>().SymbolString = InCppTypeSymbol;
		return this->GetSelf();
	}
	
	template <typename T>
	FORCEINLINE TInherited& TickSource()
	{
		const std::string_view TypeName = nameof(T);
		TickSource(FString(TypeName.data()));
		
		return this->GetSelf();
	}
	
	FORCEINLINE TInherited& Rate(const uint32 InRate)
	{
		GetSystemDefinition().Rate = InRate;
		return this->GetSelf();
	}
	
	FORCEINLINE TInherited& Rate(const TSubclassOf<UFlecsSystemObject> InTickSource, const uint32 InRate)
	{
		TickSource(InTickSource);
		Rate(InRate);
		
		return this->GetSelf();
	}
	
	FORCEINLINE TInherited& Rate(const FFlecsId InTickSource, const uint32 InRate)
	{
		TickSource(InTickSource);
		Rate(InRate);
		
		return this->GetSelf();
	}
	
	FORCEINLINE TInherited& Rate(const TSolidNotNull<const UScriptStruct*> InTickSource, const uint32 InRate)
	{
		TickSource(InTickSource);
		Rate(InRate);
		
		return this->GetSelf();
	}
	
	FORCEINLINE TInherited& Rate(const FString& InCppTypeSymbolTickSource, const uint32 InRate)
	{
		TickSource(InCppTypeSymbolTickSource);
		Rate(InRate);
		
		return this->GetSelf();
	}
	
	template <typename T>
	FORCEINLINE TInherited& Rate(const uint32 InRate)
	{
		TickSource<T>();
		Rate(InRate);
		
		return this->GetSelf();
	}
	
	template <typename Func>
	THandleType run(Func&& func) {
		using Delegate = typename flecs::_::run_delegate<
			typename std::decay<Func>::type>;

		auto ctx = FLECS_NEW(Delegate)(FLECS_FWD(func));
		GetSystemDefinition().run = Delegate::run;
		GetSystemDefinition().run_ctx = ctx;
		GetSystemDefinition().run_ctx_free = flecs::_::free_obj<Delegate>;
		return this->GetSelf().CreateRunSystem();
	}

	template <typename Func, typename EachFunc>
	THandleType run(Func&& func, EachFunc&& each_func) {
		using Delegate = typename flecs::_::run_delegate<
			typename std::decay<Func>::type>;

		auto ctx = FLECS_NEW(Delegate)(FLECS_FWD(func));
		GetSystemDefinition().run = Delegate::run;
		GetSystemDefinition().run_ctx = ctx;
		GetSystemDefinition().run_ctx_free = flecs::_::free_obj<Delegate>;
		return each(FLECS_FWD(each_func));
	}

	template <typename Func>
	THandleType each(Func&& func) {
		using Delegate = typename flecs::_::each_delegate<
			typename std::decay<Func>::type, TComponents...>;
		auto ctx = FLECS_NEW(Delegate)(FLECS_FWD(func));
		GetSystemDefinition().callback = Delegate::run;
		GetSystemDefinition().callback_ctx = ctx;
		GetSystemDefinition().callback_ctx_free = flecs::_::free_obj<Delegate>;
		return this->GetSelf().CreateEachSystem();
	}

	template <typename Func>
	THandleType run_each(Func&& func) {
		using Delegate = typename flecs::_::each_delegate<
			typename std::decay<Func>::type, TComponents...>;
		auto ctx = FLECS_NEW(Delegate)(FLECS_FWD(func));
		GetSystemDefinition().run = Delegate::run_each;
		GetSystemDefinition().run_ctx = ctx;
		GetSystemDefinition().run_ctx_free = flecs::_::free_obj<Delegate>;
		return this->GetSelf().CreateRunEachSystem();
	}
	
}; // struct TFlecsSystemBuilderBase

