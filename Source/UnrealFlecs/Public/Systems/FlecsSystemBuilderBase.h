// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "FlecsPhasesType.h"

#include "SolidMacros/Macros.h"

#include "Queries/FlecsQueryBuilderBase.h"
#include "FlecsSystemDefinition.h"

/**
 * @brief CRTP extension of the query builder for Flecs systems.
 *
 * This base adds phase, pipeline-input, scheduling, tick-source, rate, and
 * callback configuration to TFlecsQueryBuilderBase. A system is materialized
 * when one of the callback methods run(), each(), or run_each() is called.
 *
 * @tparam TInherited Concrete system builder type used for fluent returns.
 * @tparam THandleType Handle returned after the system is materialized.
 * @tparam TComponents Component field types exposed to each callbacks.
 * @see https://www.flecs.dev/flecs/Systems.html
 */
template <typename TInherited, typename THandleType, typename ...TComponents>
struct TFlecsSystemBuilderBase : public TFlecsQueryBuilderBase<TInherited>
{
public:
	/** Returns the mutable system definition owned by the derived builder. */
	FORCEINLINE FFlecsSystemDefinition& GetSystemDefinition() const
	{
		return this->GetSelf().GetSystemDefinition_Impl();
	}
	
	/** Returns the query definition embedded in the system definition. */
	FORCEINLINE FFlecsQueryDefinition& GetQueryDefinition_Impl() const
	{
		return const_cast<FFlecsQueryDefinition&>(GetSystemDefinition().QueryDefinition);
	}
	
	/**
	 * @name Phase and pipeline configuration
	 * @brief Selects the system phase and the pipeline input that gates execution.
	 * @{
	 */
	/**
	 * @brief Sets the system phase or kind from a Flecs phase input.
	 *
	 * The overload family accepts a phase enum, Flecs id, reflected struct,
	 * reflected enum selector, or exact C++ type symbol.
	 *
	 * @param InKind Phase entity or type that owns the system.
	 * @return The derived builder for fluent chaining.
	 */
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
	
	FORCEINLINE TInherited& Kind(const TSubclassOf<UObject>& InKind)
	{
		const FString ClassSymbol = InKind.Get()->GetPrefixCPP() + InKind.Get()->GetName();
		return Kind(ClassSymbol);
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

	/**
	 * @brief Alias for Kind() that sets the system phase.
	 *
	 * @param InPhase Phase entity or type that owns the system.
	 * @return The derived builder for fluent chaining.
	 */
	FORCEINLINE TInherited& Phase(const EFlecsPhaseType InPhase)
	{
		return Kind(InPhase);
	}

	FORCEINLINE TInherited& Phase(const FFlecsId InPhase)
	{
		return Kind(InPhase);
	}

	FORCEINLINE TInherited& Phase(const TSolidNotNull<const UScriptStruct*> InPhase)
	{
		return Kind(InPhase);
	}
	
	FORCEINLINE TInherited& Phase(const TSubclassOf<UObject>& InPhase)
	{
		return Kind(InPhase);
	}

	FORCEINLINE TInherited& Phase(const FSolidEnumSelector& InPhase)
	{
		return Kind(InPhase);
	}

	FORCEINLINE TInherited& Phase(const FString& InCppTypeSymbol)
	{
		return Kind(InCppTypeSymbol);
	}

	template <typename T>
	FORCEINLINE TInherited& Phase()
	{
		return Kind<T>();
	}

	/**
	 * @brief Sets the pipeline input that gates system execution.
	 *
	 * The overload family accepts a complete pipeline-input definition,
	 * gameplay tag, Flecs id, reflected struct or enum selector, or exact C++
	 * type symbol.
	 *
	 * @param InPipelineInput Pipeline input definition.
	 * @return The derived builder for fluent chaining.
	 */
	FORCEINLINE TInherited& PipelineInput(const FFlecsSystemPipelineInput& InPipelineInput)
	{
		GetSystemDefinition().PipelineInput = InPipelineInput;
		return this->GetSelf();
	}

	FORCEINLINE TInherited& PipelineInput(const FGameplayTag& InTag)
	{
		GetSystemDefinition().PipelineInput.InputType = EFlecsSystemPipelineInputType::Tag;
		GetSystemDefinition().PipelineInput.Tag = InTag;
		return this->GetSelf();
	}

	FORCEINLINE TInherited& PipelineInput(const FFlecsQueryGeneratorInput& InTypeInput)
	{
		GetSystemDefinition().PipelineInput.InputType = EFlecsSystemPipelineInputType::Type;
		GetSystemDefinition().PipelineInput.TypeInput = InTypeInput;
		return this->GetSelf();
	}

	FORCEINLINE TInherited& PipelineInput(const FFlecsId InTypeId)
	{
		GetSystemDefinition().PipelineInput.InputType = EFlecsSystemPipelineInputType::Type;
		GetSystemDefinition().PipelineInput.TypeInput.bPair = false;
		GetSystemDefinition().PipelineInput.TypeInput.First.InitializeAs<FFlecsQueryGeneratorInputType_FlecsId>();
		GetSystemDefinition().PipelineInput.TypeInput.First.GetMutable<FFlecsQueryGeneratorInputType_FlecsId>().FlecsId = InTypeId;
		return this->GetSelf();
	}

	FORCEINLINE TInherited& PipelineInput(const TSolidNotNull<const UScriptStruct*> InStruct)
	{
		GetSystemDefinition().PipelineInput.InputType = EFlecsSystemPipelineInputType::Type;
		GetSystemDefinition().PipelineInput.TypeInput.bPair = false;
		GetSystemDefinition().PipelineInput.TypeInput.First.InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		GetSystemDefinition().PipelineInput.TypeInput.First.GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = InStruct;
		return this->GetSelf();
	}

	FORCEINLINE TInherited& PipelineInput(const FSolidEnumSelector& InEnum)
	{
		GetSystemDefinition().PipelineInput.InputType = EFlecsSystemPipelineInputType::Type;
		GetSystemDefinition().PipelineInput.TypeInput.bPair = true;

		GetSystemDefinition().PipelineInput.TypeInput.First.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnum>();
		GetSystemDefinition().PipelineInput.TypeInput.First.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnum>().ScriptEnum = InEnum.Class;

		GetSystemDefinition().PipelineInput.TypeInput.Second.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnumConstant>();
		GetSystemDefinition().PipelineInput.TypeInput.Second.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnumConstant>().EnumValue = InEnum;

		return this->GetSelf();
	}

	FORCEINLINE TInherited& PipelineInput(const FString& InCppTypeSymbol)
	{
		GetSystemDefinition().PipelineInput.InputType = EFlecsSystemPipelineInputType::Type;
		GetSystemDefinition().PipelineInput.TypeInput.bPair = false;
		GetSystemDefinition().PipelineInput.TypeInput.First.InitializeAs<FFlecsQueryGeneratorInputType_CPPType>();
		GetSystemDefinition().PipelineInput.TypeInput.First.GetMutable<FFlecsQueryGeneratorInputType_CPPType>().SymbolString = InCppTypeSymbol;
		return this->GetSelf();
	}

	template <typename T>
	FORCEINLINE TInherited& PipelineInput()
	{
		const std::string_view TypeName = nameof(T);
		PipelineInput(FString(TypeName.data()));

		return this->GetSelf();
	}
	
	/**
	 * Allows the Flecs scheduler to divide matched entities across worker stages.
	 *
	 * The world must be configured with worker threads for this to execute in
	 * parallel. Callbacks can then run concurrently and receive a stage-backed
	 * world interface. Query access annotations coordinate ECS component access,
	 * but the callback remains responsible for synchronizing external state and
	 * must not use game-thread-only Unreal APIs. Immediate systems are always
	 * scheduled single-threaded.
	 *
	 * @see https://www.flecs.dev/flecs/Systems.html#threading
	 */
	FORCEINLINE TInherited& MultiThreaded(const bool bInMultiThreaded = true)
	{
		GetSystemDefinition().bMultiThreaded = bInMultiThreaded;
		return this->GetSelf();
	}
	
	/**
	 * Requests scheduled execution outside the world's readonly mode.
	 *
	 * This makes structural changes to other entities immediately visible, but
	 * operations on an entity in the table currently being iterated remain
	 * deferred to avoid invalidating the iterator. Immediate systems are always
	 * scheduled single-threaded. Explicit RunSystem calls use Flecs' manual run
	 * path, which opens its own defer scope.
	 *
	 * @see https://www.flecs.dev/flecs/Systems.html#immediate-systems
	 */
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
