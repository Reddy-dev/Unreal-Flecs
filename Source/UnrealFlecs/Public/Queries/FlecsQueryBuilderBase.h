// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once


#include "FlecsQueryFlags.h"
#include "Enums/FlecsQueryCache.h"
#include "Enums/FlecsQueryInOut.h"
#include "Expressions/FlecsQueryTermExpression.h"
#include "Generator/FlecsQueryGeneratorInputType.h"
#include "FlecsQueryDefinition.h"
#include "NativeGameplayTags.h"
#include "Callbacks/FlecsGroupByCallbackDefinition.h"
#include "Callbacks/FlecsOrderByCallbackDefinition.h"
#include "Expressions/FlecsQueryCascadeExpression.h"
#include "Expressions/FlecsQueryDescendingExpression.h"
#include "Expressions/FlecsQueryGroupByExpression.h"
#include "Expressions/FlecsQueryOrderByExpression.h"
#include "Expressions/FlecsQueryScriptExpression.h"
#include "Expressions/FlecsQueryUpExpression.h"

namespace UE::Flecs::Queries
{
	template <typename T>
	concept CQueryDefinitionRecordInputType = std::is_convertible<T, FFlecsId>::value
		|| std::is_convertible<T, const UScriptStruct*>::value
		|| std::is_convertible<T, FString>::value
		|| std::is_convertible<T, const UEnum*>::value
		|| std::is_convertible<T, FSolidEnumSelector>::value
		|| std::is_convertible<T, FGameplayTag>::value
		|| std::is_convertible<T, FNativeGameplayTag>::value;
	
	template <typename T>
	using TNoCVRef = std::remove_cv_t<std::remove_reference_t<T>>;

	template <typename T>
	inline constexpr bool bIsPointerV = std::is_pointer_v<TNoCVRef<T>>;

	template <typename T>
	using TComponentFromArg = std::remove_pointer_t<TNoCVRef<T>>;

	template <typename T>
	using TComponentBare = std::remove_cv_t<TComponentFromArg<T>>;

	template <typename T>
	inline constexpr bool bIsConstPointeeOrValueV =
		std::is_const_v<std::remove_reference_t<T>> || // const T / const T&
		std::is_const_v<std::remove_pointer_t<TNoCVRef<T>>>; // const T*

	template <typename T>
	inline constexpr bool bIsRefV = std::is_reference_v<T>;
	
	template <typename T>
	NO_DISCARD FORCEINLINE constexpr EFlecsQueryInOut TypeToInOut()
	{
		if constexpr (bIsConstPointeeOrValueV<T>)
		{
			return EFlecsQueryInOut::Read;
		}
		else if constexpr (bIsRefV<T>)
		{
			return EFlecsQueryInOut::ReadWrite;
		}
		else
		{
			return EFlecsQueryInOut::Default;
		}
	}
	
	template <typename T>
	NO_DISCARD FORCEINLINE constexpr EFlecsQueryOperator TypeToOperator()
	{
		if constexpr (bIsPointerV<T>)
		{
			return EFlecsQueryOperator::Optional;
		}
		else
		{
			return EFlecsQueryOperator::And;
		}
	}
	
} // namespace UE::Flecs::Queries

/**
 * @brief CRTP base for fluent Unreal-Flecs query-definition builders.
 *
 * This base stores query terms and options in the derived builder's
 * FFlecsQueryDefinition. The query, system, observer, and pipeline builders
 * share this API; their concrete types decide how the definition is
 * materialized.
 *
 * Term modifiers operate on the most recently added term, tracked by
 * LastTermIndex. Add a term with With(), Without(), or a pair helper before
 * applying operators, access modes, sources, or traversal expressions.
 *
 * @tparam TInherited Concrete builder type used for fluent return values.
 * @see https://www.flecs.dev/flecs/Queries.html
 */
template <typename TInherited>
struct TFlecsQueryBuilderBase
{
	using FInheritedType = TInherited;
	
protected:
	
	FORCEINLINE_DEBUGGABLE FInheritedType& GetSelf()
	{
		return static_cast<TInherited&>(*this);
	}
	
	FORCEINLINE_DEBUGGABLE const FInheritedType& GetSelf() const
	{
		return static_cast<const TInherited&>(*this);
	}

	FORCEINLINE_DEBUGGABLE FFlecsGroupByCallbackDefinition& GetMutableGroupByCallbackDefinition() const
	{
		TOptional<TInstancedStruct<FFlecsGroupByCallbackDefinition>>& GroupByCallbackDefinition =
			this->GetQueryDefinition().GroupByExpression.GroupByCallbackDefinition;

		if (!GroupByCallbackDefinition.IsSet())
		{
			TInstancedStruct<FFlecsGroupByCallbackDefinition> CallbackDefinition;
			CallbackDefinition.InitializeAs<FFlecsGroupByCallbackDefinition>();
			GroupByCallbackDefinition = CallbackDefinition;
		}

		return GroupByCallbackDefinition.GetValue().GetMutable<FFlecsGroupByCallbackDefinition>();
	}
	
public:
	
	/** Returns the mutable query definition owned by the derived builder. */
	FORCEINLINE_DEBUGGABLE FFlecsQueryDefinition& GetQueryDefinition() const
	{
		return this->GetSelf().GetQueryDefinition_Impl();
	}
	
	/** Index of the term targeted by the next term modifier. */
	mutable int32 LastTermIndex = -1;
	
	/**
	 * @brief Appends a pre-built term expression and selects it as the current term.
	 *
	 * @param InTerm Term expression to append.
	 * @return The derived builder for fluent chaining.
	 */
	FORCEINLINE_DEBUGGABLE FInheritedType& AddTerm(const FFlecsQueryTermExpression& InTerm)
	{
		this->GetQueryDefinition().AddQueryTerm(InTerm);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		return GetSelf();
	}

	/**
	 * @brief Selects an existing term as the current term.
	 *
	 * Subsequent term modifiers such as Oper(), In(), Src(), or Up() apply to
	 * this term.
	 *
	 * @param InTermIndex Zero-based index of the term to select.
	 * @return The derived builder for fluent chaining.
	 */
	FORCEINLINE_DEBUGGABLE FInheritedType& TermAt(const int32 InTermIndex)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		LastTermIndex = InTermIndex;
		return GetSelf();
	}

	/*FORCEINLINE_DEBUGGABLE FInheritedType& TermAt(*/
	
/**
 * @name Query-definition properties
 * @brief Configures cache behavior and native query flags.
 * @{
 */
#pragma region QueryDefinitionProperties
	
	/**
	 * @brief Selects the cache policy used when the query is built.
	 *
	 * @param InCacheType Cache policy. Default lets Flecs choose.
	 * @return The derived builder for fluent chaining.
	 */
	FORCEINLINE_DEBUGGABLE FInheritedType& Cache(const EFlecsQueryCacheType InCacheType = EFlecsQueryCacheType::Default)
	{
		this->GetQueryDefinition().CacheType = InCacheType;
		return GetSelf();
	}
	
	/**
	 * @brief Enables or disables query change detection.
	 *
	 * @param bInDetectChanges Whether the query should track changed tables.
	 * @return The derived builder for fluent chaining.
	 */
	FORCEINLINE_DEBUGGABLE FInheritedType& DetectChanges(const bool bInDetectChanges = true)
	{
		if (bInDetectChanges)
		{
			this->GetQueryDefinition().Flags |= static_cast<uint8>(EFlecsQueryFlags::DetectChanges);
		}
		else
		{
			this->GetQueryDefinition().Flags &= ~static_cast<uint8>(EFlecsQueryFlags::DetectChanges);
		}
		
		return GetSelf();
	}
	
	/**
	 * @brief Replaces the query flags with a raw bitmask.
	 *
	 * @param InFlags Query flags represented as a 32-bit mask.
	 * @return The derived builder for fluent chaining.
	 */
	FORCEINLINE_DEBUGGABLE FInheritedType& Flags(const uint32 InFlags)
	{
		this->GetQueryDefinition().Flags = InFlags;
		return GetSelf();
	}
	
	/** Replaces the query flags using the strongly typed enum value. */
	FORCEINLINE_DEBUGGABLE FInheritedType& Flags(const EFlecsQueryFlags InFlags)
	{
		this->GetQueryDefinition().Flags = static_cast<uint32>(InFlags);
		return GetSelf();
	}
	
#pragma endregion QueryDefinitionProperties
/** @} */
	
/**
 * @name Term operators
 * @brief Sets the operator for the current term.
 *
 * The current term is the most recently added term unless TermAt() was used to
 * select another term.
 * @{
 */
#pragma region TermOperatorExpressions
	
	/**
	 * @brief Sets the operator for the current term.
	 *
	 * @param InOperator Operator applied when matching the term.
	 * @return The derived builder for fluent chaining.
	 */
	FORCEINLINE_DEBUGGABLE FInheritedType& Oper(const EFlecsQueryOperator InOperator)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		
		this->GetQueryDefinition().Terms[LastTermIndex].Operator = InOperator;
		return GetSelf();
	}
	
	/** Sets the current term to the required And operator. */
	FORCEINLINE_DEBUGGABLE FInheritedType& And()
	{
		return Oper(EFlecsQueryOperator::And);
	}
	
	/** Sets the current term to the Or operator. */
	FORCEINLINE_DEBUGGABLE FInheritedType& Or()
	{
		return Oper(EFlecsQueryOperator::Or);
	}
	
	/** Sets the current term to the Not operator. */
	FORCEINLINE_DEBUGGABLE FInheritedType& Not()
	{
		return Oper(EFlecsQueryOperator::Not);
	}
	
	/** Sets the current term to the Optional operator. */
	FORCEINLINE_DEBUGGABLE FInheritedType& Optional()
	{
		return Oper(EFlecsQueryOperator::Optional);
	}
	
	/** Sets the current term to the AndFrom operator. */
	FORCEINLINE_DEBUGGABLE FInheritedType& AndFrom()
	{
		return Oper(EFlecsQueryOperator::AndFrom);
	}
	
	/** Sets the current term to the OrFrom operator. */
	FORCEINLINE_DEBUGGABLE FInheritedType& OrFrom()
	{
		return Oper(EFlecsQueryOperator::OrFrom);
	}
	
	/** Sets the current term to the NotFrom operator. */
	FORCEINLINE_DEBUGGABLE FInheritedType& NotFrom()
	{
		return Oper(EFlecsQueryOperator::NotFrom);
	}
	
#pragma endregion TermOperatorExpressions
/** @} */
	
/**
 * @name Term access modes
 * @brief Sets read/write access and staging behavior for the current term.
 *
 * In() and Out() describe direct access. Read(), Write(), and ReadWrite()
 * additionally mark the term as stage-aware for systems.
 * @{
 */
#pragma region ReadWriteInOutExpressions
	
	/**
	 * @brief Sets the access mode and optional stage-awareness of the current term.
	 *
	 * @param InInOut Access mode to record.
	 * @param bStage Whether the term uses stage-aware access.
	 * @return The derived builder for fluent chaining.
	 */
	FORCEINLINE_DEBUGGABLE FInheritedType& InOutExpression(const EFlecsQueryInOut InInOut, const bool bStage = false)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		
		this->GetQueryDefinition().Terms[LastTermIndex].InOut = InInOut;
		this->GetQueryDefinition().Terms[LastTermIndex].bStage = bStage;
		return GetSelf();
	}
	
	/** Marks the current term as read-only direct access. */
	FORCEINLINE_DEBUGGABLE FInheritedType& In()
	{
		return InOutExpression(EFlecsQueryInOut::Read, false);
	}
	
	/** Marks the current term as write-only direct access. */
	FORCEINLINE_DEBUGGABLE FInheritedType& Out()
	{
		return InOutExpression(EFlecsQueryInOut::Write, false);
	}
	
	/** Marks the current term as read/write direct access. */
	FORCEINLINE_DEBUGGABLE FInheritedType& InOut()
	{
		return InOutExpression(EFlecsQueryInOut::ReadWrite, false);
	}
	
	/** Marks the current term as stage-aware read access. */
	FORCEINLINE_DEBUGGABLE FInheritedType& Read()
	{
		return InOutExpression(EFlecsQueryInOut::Read, true);
	}
	
	/** Marks the current term as stage-aware write access. */
	FORCEINLINE_DEBUGGABLE FInheritedType& Write()
	{
		return InOutExpression(EFlecsQueryInOut::Write, true);
	}
	
	/** Marks the current term as stage-aware read/write access. */
	FORCEINLINE_DEBUGGABLE FInheritedType& ReadWrite()
	{
		return InOutExpression(EFlecsQueryInOut::ReadWrite, true);
	}
	
	/** Marks the current term as a filter that does not access component data. */
	FORCEINLINE_DEBUGGABLE FInheritedType& Filter()
	{
		return InOutExpression(EFlecsQueryInOut::Filter, false);
	}
	
	/** Marks the current term as neither read nor write access. */
	FORCEINLINE_DEBUGGABLE FInheritedType& InOutNone()
	{
		return InOutExpression(EFlecsQueryInOut::None, false);
	}
	
#pragma endregion ReadWriteInOutExpressions
/** @} */
	
/**
 * @name Term and query-expression helpers
 * @brief Adds terms, pair terms, ordering, grouping, sources, and traversal
 * expressions to the query definition.
 * @{
 */
#pragma region TermHelperFunctions
	
private:
	FORCEINLINE_DEBUGGABLE FInheritedType& WithCppType_Internal(const std::string_view TypeName)
	{
		const FString TypeNameFString = FString(TypeName.data());
		
		FFlecsQueryTermExpression Expr;
		Expr.Term.Input.First.InitializeAs<FFlecsQueryGeneratorInputType_CPPType>();
		Expr.Term.Input.First.GetMutable<FFlecsQueryGeneratorInputType_CPPType>().SymbolString = TypeNameFString;
		
		this->GetQueryDefinition().AddQueryTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();

		return GetSelf();
	}

	FORCEINLINE_DEBUGGABLE FInheritedType& WithoutCppType_Internal(const std::string_view TypeName)
	{
		WithCppType_Internal(TypeName);
		this->Not();
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& SecondCppType_Internal(const std::string_view TypeName)
	{
		const FString TypeNameFString = FString(TypeName.data());
		
		FFlecsQueryTermExpression& TermExpr = this->GetQueryDefinition().Terms[LastTermIndex];
		TermExpr.Term.Input.bPair = true;
		TermExpr.Term.Input.Second.InitializeAs<FFlecsQueryGeneratorInputType_CPPType>();
		TermExpr.Term.Input.Second.GetMutable<FFlecsQueryGeneratorInputType_CPPType>().SymbolString = TypeNameFString;
		
		return GetSelf();
	}
	
public:
	
	/**
	 * @brief Adds a term that must match.
	 *
	 * The overload family accepts a Flecs id, reflected struct or enum, C++
	 * type symbol, gameplay tag, or native gameplay tag. The added term becomes
	 * the current term for subsequent modifiers. A Flecs pair id can be passed
	 * directly; use Second() or WithPair() when the pair parts are separate.
	 *
	 * @param InId Component, tag, or pair id to match.
	 * @return The derived builder for fluent chaining.
	 */
	FORCEINLINE_DEBUGGABLE FInheritedType& With(const FFlecsId InId)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.Input.First.InitializeAs<FFlecsQueryGeneratorInputType_FlecsId>();
		Expr.Term.Input.First.GetMutable<FFlecsQueryGeneratorInputType_FlecsId>().FlecsId = InId;
		
		this->GetQueryDefinition().AddQueryTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();

		return GetSelf();
	}

	FORCEINLINE_DEBUGGABLE FInheritedType& With(const TSolidNotNull<const UScriptStruct*> InStruct)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.Input.First.InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		Expr.Term.Input.First.GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = InStruct;
		
		this->AddTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& With(const FString& InString)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.Input.First.InitializeAs<FFlecsQueryGeneratorInputType_String>();
		Expr.Term.Input.First.GetMutable<FFlecsQueryGeneratorInputType_String>().InputString = InString;
		
		this->GetQueryDefinition().AddQueryTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& With(const TSolidNotNull<const UEnum*> InEnum)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.Input.First.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnum>();
		Expr.Term.Input.First.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnum>().ScriptEnum = InEnum;
		
		this->GetQueryDefinition().AddQueryTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& With(const FSolidEnumSelector& InEnumSelector)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.Input.First.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnumConstant>();
		Expr.Term.Input.First.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnumConstant>().EnumValue = InEnumSelector;
		
		this->GetQueryDefinition().AddQueryTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& With(const FGameplayTag& InGameplayTag)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.Input.First.InitializeAs<FFlecsQueryGeneratorInputType_GameplayTag>();
		Expr.Term.Input.First.GetMutable<FFlecsQueryGeneratorInputType_GameplayTag>().GameplayTag = InGameplayTag;
		
		this->GetQueryDefinition().AddQueryTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& With(const FNativeGameplayTag& InNativeGameplayTag)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.Input.First.InitializeAs<FFlecsQueryGeneratorInputType_GameplayTag>();
		Expr.Term.Input.First.GetMutable<FFlecsQueryGeneratorInputType_GameplayTag>().GameplayTag = InNativeGameplayTag.GetTag();
		
		this->GetQueryDefinition().AddQueryTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		return GetSelf();
	}
	
	/**
	 * @brief Adds a term from a C++ type.
	 *
	 * @tparam T Component, enum, or other registered type to match.
	 * @return The derived builder for fluent chaining.
	 */
	template <typename T>
	FORCEINLINE_DEBUGGABLE FInheritedType& With()
	{
		if constexpr (Solid::IsScriptStruct<T>())
		{
			this->With(TBaseStructure<T>::Get());
			this->InOutExpression(UE::Flecs::Queries::TypeToInOut<T>(), false);
		}
		else if constexpr (Solid::TStaticEnumConcept<T>)
		{
			this->With(StaticEnum<T>());
		}
		else if constexpr (std::is_enum<T>::value)
		{
			FFlecsQueryTermExpression Expr;
			Expr.Term.Input.bPair = true;
			Expr.Term.Input.First.InitializeAs<FFlecsQueryGeneratorInputType_CPPEnum>();
			Expr.Term.Input.First.GetMutable<FFlecsQueryGeneratorInputType_CPPEnum>().SymbolString = FString(nameof(T).data());
			Expr.Term.Input.Second.InitializeAs<FFlecsQueryGeneratorInputType_Wildcard>();

			this->GetQueryDefinition().AddQueryTerm(Expr);
			LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		}
		else
		{
			using TNoCVRef = std::remove_cv_t<std::remove_reference_t<T>>;
			
			const std::string_view TypeName = nameof(TNoCVRef);
			this->WithCppType_Internal(TypeName);
			this->InOutExpression(UE::Flecs::Queries::TypeToInOut<T>(), false);
		}
		
		return GetSelf();
	}

	/** Adds a pair term for an enum value. */
	template <typename E>
	requires (std::is_enum<E>::value)
	FORCEINLINE_DEBUGGABLE FInheritedType& With(const E InEnumValue)
	{
		this->WithPair<E>(InEnumValue);
		return GetSelf();
	}

	/**
	 * @brief Adds a term with stage-aware read access.
	 *
	 * @tparam T Input type accepted by With().
	 * @param InInput Component or id input to match.
	 * @return The derived builder for fluent chaining.
	 */
	template <UE::Flecs::Queries::CQueryDefinitionRecordInputType T>
	FORCEINLINE_DEBUGGABLE FInheritedType& Read(const T& InInput)
	{
		this->With(InInput);
		return InOutExpression(EFlecsQueryInOut::Read, true);
	}

	/** Adds a C++ type term with stage-aware read access. */
	template <typename T>
	FORCEINLINE_DEBUGGABLE FInheritedType& Read()
	{
		this->With<T>();
		return InOutExpression(EFlecsQueryInOut::Read, true);
	}

	/** Adds a term with stage-aware write access. */
	template <UE::Flecs::Queries::CQueryDefinitionRecordInputType T>
	FORCEINLINE_DEBUGGABLE FInheritedType& Write(const T& InInput)
	{
		this->With(InInput);
		return InOutExpression(EFlecsQueryInOut::Write, true);
	}

	/** Adds a C++ type term with stage-aware write access. */
	template <typename T>
	FORCEINLINE_DEBUGGABLE FInheritedType& Write()
	{
		this->With<T>();
		return InOutExpression(EFlecsQueryInOut::Write, true);
	}

	/** Adds a term with stage-aware read/write access. */
	template <UE::Flecs::Queries::CQueryDefinitionRecordInputType T>
	FORCEINLINE_DEBUGGABLE FInheritedType& ReadWrite(const T& InInput)
	{
		this->With(InInput);
		return InOutExpression(EFlecsQueryInOut::ReadWrite, true);
	}

	/** Adds a C++ type term with stage-aware read/write access. */
	template <typename T>
	FORCEINLINE_DEBUGGABLE FInheritedType& ReadWrite()
	{
		this->With<T>();
		return InOutExpression(EFlecsQueryInOut::ReadWrite, true);
	}
	
	/**
	 * @brief Adds a term that must not match.
	 *
	 * The overload family accepts the same reflected and runtime input forms
	 * as With(). The added term is automatically assigned the Not operator.
	 *
	 * @param InId Component, tag, or pair id to exclude.
	 * @return The derived builder for fluent chaining.
	 */
	FORCEINLINE_DEBUGGABLE FInheritedType& Without(const FFlecsId InId)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.Input.First.InitializeAs<FFlecsQueryGeneratorInputType_FlecsId>();
		Expr.Term.Input.First.GetMutable<FFlecsQueryGeneratorInputType_FlecsId>().FlecsId = InId;
		
		this->AddTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		this->Not();
		
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Without(const TSolidNotNull<const UScriptStruct*> InStruct)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.Input.First.InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		Expr.Term.Input.First.GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = InStruct;
		
		this->AddTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		this->Not();
		
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Without(const FString& InString)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.Input.First.InitializeAs<FFlecsQueryGeneratorInputType_String>();
		Expr.Term.Input.First.GetMutable<FFlecsQueryGeneratorInputType_String>().InputString = InString;
		
		this->AddTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		this->Not();
		
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Without(const TSolidNotNull<const UEnum*> InEnum)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.Input.First.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnum>();
		Expr.Term.Input.First.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnum>().ScriptEnum = InEnum;
		
		this->AddTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		this->Not();
		
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Without(const FSolidEnumSelector& InEnumSelector)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.Input.First.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnumConstant>();
		Expr.Term.Input.First.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnumConstant>().EnumValue = InEnumSelector;
		
		this->AddTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		this->Not();
		
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Without(const FGameplayTag& InGameplayTag)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.Input.First.InitializeAs<FFlecsQueryGeneratorInputType_GameplayTag>();
		Expr.Term.Input.First.GetMutable<FFlecsQueryGeneratorInputType_GameplayTag>().GameplayTag = InGameplayTag;
		
		this->AddTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		this->Not();
		
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Without(const FNativeGameplayTag& InNativeGameplayTag)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.Input.First.InitializeAs<FFlecsQueryGeneratorInputType_GameplayTag>();
		Expr.Term.Input.First.GetMutable<FFlecsQueryGeneratorInputType_GameplayTag>().GameplayTag = InNativeGameplayTag.GetTag();
		
		this->AddTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		this->Not();
		
		return GetSelf();
	}
	
	/** Adds a C++ type term with the Not operator. */
	template <typename T>
	FORCEINLINE_DEBUGGABLE FInheritedType& Without()
	{
		if constexpr (Solid::IsScriptStruct<T>())
		{
			this->Without(TBaseStructure<T>::Get());
		}
		else if constexpr (Solid::TStaticEnumConcept<T>)
		{
			this->Without(StaticEnum<T>());
		}
		else if constexpr (std::is_enum<T>::value)
		{
			this->With<T>();
			this->Not();
		}
		else
		{
			const std::string_view TypeName = nameof(T);
			this->WithoutCppType_Internal(TypeName);
		}
		
		return GetSelf();
	}
	
	/** Adds an enum-value pair term with the Not operator. */
	template <typename E>
	requires (std::is_enum<E>::value)
	FORCEINLINE_DEBUGGABLE FInheritedType& Without(const E InEnumValue)
	{
		this->template WithoutPair<E>(InEnumValue);
		return GetSelf();
	}
	
	/**
	 * @brief Sets the second element of the current term as a pair target.
	 *
	 * Call this after With() or Without(). The overload family accepts Flecs
	 * ids, reflected types and enums, C++ type symbols, gameplay tags, and
	 * native gameplay tags.
	 *
	 * @param InId Pair target id.
	 * @return The derived builder for fluent chaining.
	 */
	FORCEINLINE_DEBUGGABLE FInheritedType& Second(const FFlecsId InId)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		
		FFlecsQueryTermExpression& TermExpr = this->GetQueryDefinition().Terms[LastTermIndex];
		TermExpr.Term.Input.bPair = true;
		TermExpr.Term.Input.Second.InitializeAs<FFlecsQueryGeneratorInputType_FlecsId>();
		TermExpr.Term.Input.Second.GetMutable<FFlecsQueryGeneratorInputType_FlecsId>().FlecsId = InId;
		
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Second(const TSolidNotNull<const UScriptStruct*> InStruct)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		
		FFlecsQueryTermExpression& TermExpr = this->GetQueryDefinition().Terms[LastTermIndex];
		TermExpr.Term.Input.bPair = true;
		TermExpr.Term.Input.Second.InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		TermExpr.Term.Input.Second.GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = InStruct;
		
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Second(const FString& InString)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		
		FFlecsQueryTermExpression& TermExpr = this->GetQueryDefinition().Terms[LastTermIndex];
		TermExpr.Term.Input.bPair = true;
		TermExpr.Term.Input.Second.InitializeAs<FFlecsQueryGeneratorInputType_String>();
		TermExpr.Term.Input.Second.GetMutable<FFlecsQueryGeneratorInputType_String>().InputString = InString;
		
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Second(const TSolidNotNull<const UEnum*> InEnum)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		
		FFlecsQueryTermExpression& TermExpr = this->GetQueryDefinition().Terms[LastTermIndex];
		TermExpr.Term.Input.bPair = true;
		TermExpr.Term.Input.Second.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnum>();
		TermExpr.Term.Input.Second.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnum>().ScriptEnum = InEnum;
		
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Second(const FSolidEnumSelector& InEnumSelector)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		
		FFlecsQueryTermExpression& TermExpr = this->GetQueryDefinition().Terms[LastTermIndex];
		TermExpr.Term.Input.bPair = true;
		TermExpr.Term.Input.Second.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnumConstant>();
		TermExpr.Term.Input.Second.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnumConstant>().EnumValue = InEnumSelector;
		
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Second(const FGameplayTag& InGameplayTag)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		
		FFlecsQueryTermExpression& TermExpr = this->GetQueryDefinition().Terms[LastTermIndex];
		TermExpr.Term.Input.bPair = true;
		TermExpr.Term.Input.Second.InitializeAs<FFlecsQueryGeneratorInputType_GameplayTag>();
		TermExpr.Term.Input.Second.GetMutable<FFlecsQueryGeneratorInputType_GameplayTag>().GameplayTag = InGameplayTag;
		
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Second(const FNativeGameplayTag& InNativeGameplayTag)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		
		FFlecsQueryTermExpression& TermExpr = this->GetQueryDefinition().Terms[LastTermIndex];
		TermExpr.Term.Input.bPair = true;
		TermExpr.Term.Input.Second.InitializeAs<FFlecsQueryGeneratorInputType_GameplayTag>();
		TermExpr.Term.Input.Second.GetMutable<FFlecsQueryGeneratorInputType_GameplayTag>().GameplayTag = InNativeGameplayTag.GetTag();
		
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Second_CppType(const FString& InTypeName)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		
		FFlecsQueryTermExpression& TermExpr = this->GetQueryDefinition().Terms[LastTermIndex];
		TermExpr.Term.Input.bPair = true;
		TermExpr.Term.Input.Second.InitializeAs<FFlecsQueryGeneratorInputType_CPPType>();
		TermExpr.Term.Input.Second.GetMutable<FFlecsQueryGeneratorInputType_CPPType>().SymbolString = InTypeName;
		
		return GetSelf();
	}
	
	/** Sets the pair target from a C++ type. */
	template <typename T>
	FORCEINLINE_DEBUGGABLE FInheritedType& Second()
	{
		if constexpr (Solid::IsScriptStruct<T>())
		{
			this->Second(TBaseStructure<T>::Get());
		}
		else if constexpr (Solid::TStaticEnumConcept<T>)
		{
			this->Second(StaticEnum<T>());
		}
		else if constexpr (std::is_enum<T>::value)
		{
			solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));

			FFlecsQueryTermExpression& TermExpr = this->GetQueryDefinition().Terms[LastTermIndex];
			TermExpr.Term.Input.bPair = true;
			TermExpr.Term.Input.Second.InitializeAs<FFlecsQueryGeneratorInputType_CPPEnum>();
			TermExpr.Term.Input.Second.GetMutable<FFlecsQueryGeneratorInputType_CPPEnum>().SymbolString = FString(nameof(T).data());
		}
		else
		{
			using TNoCVRef = std::remove_cv_t<std::remove_reference_t<T>>;
			const std::string_view TypeName = nameof(TNoCVRef);
			const FString TypeNameFString = FString(TypeName.data());
			this->Second_CppType(TypeNameFString);
		}
		
		return GetSelf();
	}
	
	/**
	 * @brief Adds a matching pair term from two runtime or reflected inputs.
	 *
	 * @tparam TFirst Pair relation input type.
	 * @tparam TSecond Pair target input type.
	 * @param InFirst Pair relation.
	 * @param InSecond Pair target.
	 * @return The derived builder for fluent chaining.
	 */
	template <UE::Flecs::Queries::CQueryDefinitionRecordInputType TFirst, UE::Flecs::Queries::CQueryDefinitionRecordInputType TSecond>
	FORCEINLINE_DEBUGGABLE FInheritedType& WithPair(const TFirst& InFirst, const TSecond& InSecond)
	{
		this->With(InFirst);
		this->Second(InSecond);
		return GetSelf();
	}
	
	/** Adds a pair term with the Not operator. */
	template <UE::Flecs::Queries::CQueryDefinitionRecordInputType TFirst, UE::Flecs::Queries::CQueryDefinitionRecordInputType TSecond>
	FORCEINLINE_DEBUGGABLE FInheritedType& WithoutPair(const TFirst& InFirst, const TSecond& InSecond)
	{
		this->Without(InFirst);
		this->Second(InSecond);
		return GetSelf();
	}
	
	/** Adds a pair with the relation supplied as a C++ type. */
	template <typename T, UE::Flecs::Queries::CQueryDefinitionRecordInputType TSecond>
	FORCEINLINE_DEBUGGABLE FInheritedType& WithPair(const TSecond& InSecond)
	{
		this->With<T>();
		this->Second(InSecond);
		return GetSelf();
	}
	
	/** Adds a negated pair with the relation supplied as a C++ type. */
	template <typename T, UE::Flecs::Queries::CQueryDefinitionRecordInputType TSecond>
	FORCEINLINE_DEBUGGABLE FInheritedType& WithoutPair(const TSecond& InSecond)
	{
		this->Without<T>();
		this->Second(InSecond);
		return GetSelf();
	}
	
	/** Adds a pair with the target supplied as a C++ type. */
	template <typename T, UE::Flecs::Queries::CQueryDefinitionRecordInputType TFirst>
	FORCEINLINE_DEBUGGABLE FInheritedType& WithPairSecond(const TFirst& InFirst)
	{
		this->With(InFirst);
		this->Second<T>();
		return GetSelf();
	}
	
	/** Adds a negated pair with the target supplied as a C++ type. */
	template <typename T, UE::Flecs::Queries::CQueryDefinitionRecordInputType TFirst>
	FORCEINLINE_DEBUGGABLE FInheritedType& WithoutPairSecond(const TFirst& InFirst)
	{
		this->Without(InFirst);
		this->Second<T>();
		return GetSelf();
	}
	
	/** Adds a pair from two compile-time C++ types. */
	template <typename TFirst, typename TSecond>
	FORCEINLINE_DEBUGGABLE FInheritedType& WithPair()
	{
		this->With<TFirst>();
		this->Second<TSecond>();
		return GetSelf();
	}
	
	/** Adds a pair whose target is an enum constant. */
	template <typename E>
	requires (std::is_enum<E>::value)
	FORCEINLINE_DEBUGGABLE FInheritedType& WithPair(const E InEnumValue)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.Input.bPair = true;
		Expr.Term.Input.First.InitializeAs<FFlecsQueryGeneratorInputType_CPPEnum>();
		Expr.Term.Input.First.GetMutable<FFlecsQueryGeneratorInputType_CPPEnum>().SymbolString = FString(nameof(E).data());
		Expr.Term.Input.Second.InitializeAs<FFlecsQueryGeneratorInputType_CPPEnumConstant>();
		FFlecsQueryGeneratorInputType_CPPEnumConstant& EnumConstant =
			Expr.Term.Input.Second.GetMutable<FFlecsQueryGeneratorInputType_CPPEnumConstant>();
		EnumConstant.EnumSymbolString = FString(nameof(E).data());
		EnumConstant.EnumValue = static_cast<int64>(InEnumValue);

		this->GetQueryDefinition().AddQueryTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();

		return GetSelf();
	}

	/** Adds a negated pair from two compile-time C++ types. */
	template <typename TFirst, typename TSecond>
	FORCEINLINE_DEBUGGABLE FInheritedType& WithoutPair()
	{
		this->Without<TFirst>();
		this->Second<TSecond>();
		return GetSelf();
	}
	
	/** Adds a pair from a reflected enum selector. */
	FORCEINLINE_DEBUGGABLE FInheritedType& WithPair(const FSolidEnumSelector& InPair)
	{
		WithPair(InPair.Class, InPair.Value);
		return GetSelf();
	}
	
	/** Adds a negated pair from a reflected enum selector. */
	FORCEINLINE_DEBUGGABLE FInheritedType& WithoutPair(const FSolidEnumSelector& InPair)
	{
		WithoutPair(InPair.Class, InPair.Value);
		return GetSelf();
	}
	
#pragma endregion TermHelperFunctions
/** @} */
	
/**
 * @name Ordering
 * @brief Adds an order-by expression to the query definition.
 * @{
 */
#pragma region OrderByFunctions
	
	/**
	 * @brief Orders matched tables using a comparison callback.
	 *
	 * The overload family accepts a Flecs id, reflected type, enum, or exact
	 * C++ type symbol. The callback compares two component values and returns
	 * the ordering result expected by Flecs.
	 *
	 * @param InId Component id used for ordering.
	 * @param InFunction Comparison callback.
	 * @return The derived builder for fluent chaining.
	 * @see https://www.flecs.dev/flecs/Queries.html#order-by
	 */
	FORCEINLINE_DEBUGGABLE FInheritedType& OrderBy(const FFlecsId InId, UE::Flecs::Queries::FOrderByFunctionType InFunction)
	{
		TInstancedStruct<FFlecsQueryExpression> OrderByExpr;
		OrderByExpr.InitializeAs<FFlecsQueryOrderByCPPExpressionWrapper>();
		OrderByExpr.GetMutable<FFlecsQueryOrderByCPPExpressionWrapper>().OrderByInput.InitializeAs<FFlecsQueryGeneratorInputType_FlecsId>();
		OrderByExpr.GetMutable<FFlecsQueryOrderByCPPExpressionWrapper>().OrderByInput.GetMutable<FFlecsQueryGeneratorInputType_FlecsId>().FlecsId = InId;
		OrderByExpr.GetMutable<FFlecsQueryOrderByCPPExpressionWrapper>().OrderByFunction = InFunction;
		
		this->GetQueryDefinition().OtherExpressions.Add(OrderByExpr);
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& OrderBy(const UScriptStruct* InStruct, UE::Flecs::Queries::FOrderByFunctionType InFunction)
	{
		TInstancedStruct<FFlecsQueryExpression> OrderByExpr;
		OrderByExpr.InitializeAs<FFlecsQueryOrderByCPPExpressionWrapper>();
		OrderByExpr.GetMutable<FFlecsQueryOrderByCPPExpressionWrapper>().OrderByInput.InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		OrderByExpr.GetMutable<FFlecsQueryOrderByCPPExpressionWrapper>().OrderByInput.GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = InStruct;
		OrderByExpr.GetMutable<FFlecsQueryOrderByCPPExpressionWrapper>().OrderByFunction = InFunction;
		
		this->GetQueryDefinition().OtherExpressions.Add(OrderByExpr);
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& OrderBy(const UEnum* InEnum, const UE::Flecs::Queries::FOrderByFunctionType& InFunction)
	{
		TInstancedStruct<FFlecsQueryExpression> OrderByExpr;
		OrderByExpr.InitializeAs<FFlecsQueryOrderByExpression>();
		OrderByExpr.GetMutable<FFlecsQueryOrderByCPPExpressionWrapper>().OrderByInput.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnum>();
		OrderByExpr.GetMutable<FFlecsQueryOrderByCPPExpressionWrapper>().OrderByInput.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnum>().ScriptEnum = InEnum;
		OrderByExpr.GetMutable<FFlecsQueryOrderByCPPExpressionWrapper>().OrderByFunction = InFunction;
		
		this->GetQueryDefinition().OtherExpressions.Add(OrderByExpr);
		return GetSelf();
	}
	
	// Note: The InCppTypeName should be the exact match of what the EcsSymbol would be for the given C++ type.
	FORCEINLINE_DEBUGGABLE FInheritedType& OrderBy(const FString& InCppTypeName, const UE::Flecs::Queries::FOrderByFunctionType& InFunction)
	{
		TInstancedStruct<FFlecsQueryExpression> OrderByExpr;
		OrderByExpr.InitializeAs<FFlecsQueryOrderByCPPExpressionWrapper>();
		OrderByExpr.GetMutable<FFlecsQueryOrderByCPPExpressionWrapper>().OrderByInput.InitializeAs<FFlecsQueryGeneratorInputType_CPPType>();
		OrderByExpr.GetMutable<FFlecsQueryOrderByCPPExpressionWrapper>().OrderByInput.GetMutable<FFlecsQueryGeneratorInputType_CPPType>().SymbolString = InCppTypeName;
		OrderByExpr.GetMutable<FFlecsQueryOrderByCPPExpressionWrapper>().OrderByFunction = InFunction;
		
		this->GetQueryDefinition().OtherExpressions.Add(OrderByExpr);
		return GetSelf();
	}
	
	/** Adds an order-by expression for a C++ type and typed callback. */
	template <typename T>
	FORCEINLINE_DEBUGGABLE FInheritedType& OrderBy(const UE::Flecs::Queries::TOrderByFunction<T>& InFunction)
	{
		if constexpr (Solid::IsScriptStruct<T>())
		{
			return OrderBy(TBaseStructure<T>::Get(), UE::Flecs::Queries::FOrderByFunctionType(InFunction));
		}
		else if constexpr (Solid::TStaticEnumConcept<T>)
		{
			return OrderBy(StaticEnum<T>(), UE::Flecs::Queries::FOrderByFunctionType(InFunction));
		}
		else
		{
			const std::string_view TypeName = nameof(T);
			return OrderBy(FString(TypeName.data()), 
				UE::Flecs::Queries::FOrderByFunctionType(InFunction));
		}
	}
	
	/**
	 * @brief Adds an order-by expression backed by a reflected callback definition.
	 *
	 * @param InId Component id used for ordering.
	 * @param InCallbackDefinition Callback definition to store.
	 * @return The derived builder for fluent chaining.
	 */
	FORCEINLINE_DEBUGGABLE FInheritedType& OrderByCallbackDefinition(const FFlecsId InId, const TInstancedStruct<FFlecsOrderByCallbackDefinition>& InCallbackDefinition)
	{
		TInstancedStruct<FFlecsQueryExpression> OrderByExpr;
		OrderByExpr.InitializeAs<FFlecsQueryOrderByExpression>();
		OrderByExpr.GetMutable<FFlecsQueryOrderByExpression>().OrderByInput.InitializeAs<FFlecsQueryGeneratorInputType_FlecsId>();
		OrderByExpr.GetMutable<FFlecsQueryOrderByExpression>().OrderByInput.GetMutable<FFlecsQueryGeneratorInputType_FlecsId>().FlecsId = InId;
		OrderByExpr.GetMutable<FFlecsQueryOrderByExpression>().OrderByCallback = InCallbackDefinition;
		
		this->GetQueryDefinition().OtherExpressions.Add(OrderByExpr);
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& OrderByCallbackDefinition(const UScriptStruct* InStruct, const TInstancedStruct<FFlecsOrderByCallbackDefinition>& InCallbackDefinition)
	{
		TInstancedStruct<FFlecsQueryExpression> OrderByExpr;
		OrderByExpr.InitializeAs<FFlecsQueryOrderByExpression>();
		OrderByExpr.GetMutable<FFlecsQueryOrderByExpression>().OrderByInput.InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		OrderByExpr.GetMutable<FFlecsQueryOrderByExpression>().OrderByInput.GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = InStruct;
		OrderByExpr.GetMutable<FFlecsQueryOrderByExpression>().OrderByCallback = InCallbackDefinition;
		
		this->GetQueryDefinition().OtherExpressions.Add(OrderByExpr);
		return GetSelf();
	}
	
	// Note: The InCppTypeName should be the exact match of what the EcsSymbol would be for the given C++ type.
	FORCEINLINE_DEBUGGABLE FInheritedType& OrderByCallbackDefinition(const FString& InCppTypeName, const TInstancedStruct<FFlecsOrderByCallbackDefinition>& InCallbackDefinition)
	{
		TInstancedStruct<FFlecsQueryExpression> OrderByExpr;
		OrderByExpr.InitializeAs<FFlecsQueryOrderByExpression>();
		OrderByExpr.GetMutable<FFlecsQueryOrderByExpression>().OrderByInput.InitializeAs<FFlecsQueryGeneratorInputType_CPPType>();
		OrderByExpr.GetMutable<FFlecsQueryOrderByExpression>().OrderByInput.GetMutable<FFlecsQueryGeneratorInputType_CPPType>().SymbolString = InCppTypeName;
		OrderByExpr.GetMutable<FFlecsQueryOrderByExpression>().OrderByCallback = InCallbackDefinition;
		
		this->GetQueryDefinition().OtherExpressions.Add(OrderByExpr);
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& OrderByCallbackDefinition(const UEnum* InEnum, const TInstancedStruct<FFlecsOrderByCallbackDefinition>& InCallbackDefinition)
	{
		TInstancedStruct<FFlecsQueryExpression> OrderByExpr;
		OrderByExpr.InitializeAs<FFlecsQueryOrderByExpression>();
		OrderByExpr.GetMutable<FFlecsQueryOrderByExpression>().OrderByInput.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnum>();
		OrderByExpr.GetMutable<FFlecsQueryOrderByExpression>().OrderByInput.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnum>().ScriptEnum = InEnum;
		OrderByExpr.GetMutable<FFlecsQueryOrderByExpression>().OrderByCallback = InCallbackDefinition;
		
		this->GetQueryDefinition().OtherExpressions.Add(OrderByExpr);
		return GetSelf();
	}
	
	// @TODO: does this need a gameplay tag impl?
	
	template <typename T>
	FORCEINLINE_DEBUGGABLE FInheritedType& OrderByCallbackDefinition(const TInstancedStruct<FFlecsOrderByCallbackDefinition>& InCallbackDefinition)
	{
		if constexpr (Solid::IsScriptStruct<T>())
		{
			return OrderByCallbackDefinition(TBaseStructure<T>::Get(), InCallbackDefinition);
		}
		else if constexpr (Solid::TStaticEnumConcept<T>)
		{
			return OrderByCallbackDefinition(StaticEnum<T>(), InCallbackDefinition);
		}
		else
		{
			const std::string_view TypeName = nameof(T);
			return OrderByCallbackDefinition(FString(TypeName.data()), InCallbackDefinition);
		}
	}
	
#pragma endregion OrderByFunctions
/** @} */
	
/**
 * @name Grouping
 * @brief Configures table grouping and optional group lifecycle callbacks.
 * @{
 */
#pragma region GroupByFunctions
	
	/**
	 * @brief Groups matched tables by an id or type.
	 *
	 * The overload family accepts a Flecs id, reflected struct or enum, or
	 * exact C++ type symbol. Passing an invalid or empty input disables
	 * grouping.
	 *
	 * @param InId Id used to select the group value.
	 * @return The derived builder for fluent chaining.
	 * @see https://www.flecs.dev/flecs/Queries.html#group-by
	 */
	FORCEINLINE_DEBUGGABLE FInheritedType& GroupBy(const FFlecsId InId)
	{
		if (InId.IsValid())
		{
			this->GetQueryDefinition().bUseGroupBy = true;
		}
		else
		{
			this->GetQueryDefinition().bUseGroupBy = false;
			return GetSelf();
		}
		
		this->GetQueryDefinition().GroupByExpression.GroupByInput.InitializeAs<FFlecsQueryGeneratorInputType_FlecsId>();
		this->GetQueryDefinition().GroupByExpression.GroupByInput.GetMutable<FFlecsQueryGeneratorInputType_FlecsId>().FlecsId = InId;

		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& GroupBy(const UScriptStruct* InStruct)
	{
		if (InStruct != nullptr)
		{
			this->GetQueryDefinition().bUseGroupBy = true;
		}
		else
		{
			this->GetQueryDefinition().bUseGroupBy = false;
			return GetSelf();
		}
		
		this->GetQueryDefinition().GroupByExpression.GroupByInput.InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		this->GetQueryDefinition().GroupByExpression.GroupByInput.GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = InStruct;

		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& GroupBy(const UEnum* InEnum)
	{
		if (InEnum != nullptr)
		{
			this->GetQueryDefinition().bUseGroupBy = true;
		}
		else
		{
			this->GetQueryDefinition().bUseGroupBy = false;
			return GetSelf();
		}
		
		this->GetQueryDefinition().GroupByExpression.GroupByInput.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnum>();
		this->GetQueryDefinition().GroupByExpression.GroupByInput.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnum>().ScriptEnum = InEnum;

		return GetSelf();
	}
	
	// Note: The InCppTypeName should be the exact match of what the EcsSymbol would be for the given C++ type.
	FORCEINLINE_DEBUGGABLE FInheritedType& GroupBy(const FString& InCppTypeName)
	{
		if (!InCppTypeName.IsEmpty())
		{
			this->GetQueryDefinition().bUseGroupBy = true;
		}
		else
		{
			this->GetQueryDefinition().bUseGroupBy = false;
			return GetSelf();
		}
		
		this->GetQueryDefinition().GroupByExpression.GroupByInput.InitializeAs<FFlecsQueryGeneratorInputType_CPPType>();
		this->GetQueryDefinition().GroupByExpression.GroupByInput.GetMutable<FFlecsQueryGeneratorInputType_CPPType>().SymbolString = InCppTypeName;

		return GetSelf();
	}
	
	/** Groups matched tables by a C++ type. */
	template <typename T>
	FORCEINLINE_DEBUGGABLE FInheritedType& GroupBy()
	{
		const std::string_view TypeName = nameof(T);
		return GroupBy(FString(TypeName.data()));
	}
	
	/**
	 * @brief Groups matched tables with a custom group-id callback.
	 *
	 * @param InId Id used to select the group value.
	 * @param InCallbackDefinition Function that computes each group id.
	 * @return The derived builder for fluent chaining.
	 */
	FORCEINLINE_DEBUGGABLE FInheritedType& GroupBy(const FFlecsId InId, UE::Flecs::Queries::FGroupByFunctionType InCallbackDefinition)
	{
		this->GroupBy(InId);
		
		GetMutableGroupByCallbackDefinition().GroupByFunctionPtr = InCallbackDefinition;
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& GroupBy(const UScriptStruct* InStruct, UE::Flecs::Queries::FGroupByFunctionType InCallbackDefinition)
	{
		this->GroupBy(InStruct);
		
		GetMutableGroupByCallbackDefinition().GroupByFunctionPtr = InCallbackDefinition;
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& GroupBy(const UEnum* InEnum, UE::Flecs::Queries::FGroupByFunctionType InCallbackDefinition)
	{
		this->GroupBy(InEnum);
		
		GetMutableGroupByCallbackDefinition().GroupByFunctionPtr = InCallbackDefinition;
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& GroupBy(const FString& InCppTypeName, UE::Flecs::Queries::FGroupByFunctionType InCallbackDefinition)
	{
		this->GroupBy(InCppTypeName);
		
		GetMutableGroupByCallbackDefinition().GroupByFunctionPtr = InCallbackDefinition;
		return GetSelf();
	}
	
	template <typename T>
	FORCEINLINE_DEBUGGABLE FInheritedType& GroupBy(const UE::Flecs::Queries::FGroupByFunctionType InCallbackDefinition)
	{
		const std::string_view TypeName = nameof(T);
		return GroupBy(FString(TypeName.data()), UE::Flecs::Queries::FGroupByFunctionType(InCallbackDefinition));
	}
	
	/**
	 * @brief Supplies a complete reflected group callback definition.
	 *
	 * @param InId Id used to select the group value.
	 * @param InCallbackDefinition Group-id and lifecycle callbacks.
	 * @return The derived builder for fluent chaining.
	 */
	FORCEINLINE_DEBUGGABLE FInheritedType& GroupByCallbackDefinition(const FFlecsId InId, const TInstancedStruct<FFlecsGroupByCallbackDefinition>& InCallbackDefinition)
	{
		this->GroupBy(InId);
		
		this->GetQueryDefinition().GroupByExpression.GroupByCallbackDefinition = InCallbackDefinition;
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& GroupByCallbackDefinition(const UScriptStruct* InStruct, const TInstancedStruct<FFlecsGroupByCallbackDefinition>& InCallbackDefinition)
	{
		this->GroupBy(InStruct);
		
		this->GetQueryDefinition().GroupByExpression.GroupByCallbackDefinition = InCallbackDefinition;
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& GroupByCallbackDefinition(const UEnum* InEnum, const TInstancedStruct<FFlecsGroupByCallbackDefinition>& InCallbackDefinition)
	{
		this->GroupBy(InEnum);
		
		this->GetQueryDefinition().GroupByExpression.GroupByCallbackDefinition = InCallbackDefinition;
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& GroupByCallbackDefinition(const FString& InCppTypeName, const TInstancedStruct<FFlecsGroupByCallbackDefinition>& InCallbackDefinition)
	{
		this->GroupBy(InCppTypeName);
		
		this->GetQueryDefinition().GroupByExpression.GroupByCallbackDefinition = InCallbackDefinition;
		return GetSelf();
	}
	
	/** Sets the callback invoked when Flecs creates a group. */
	FORCEINLINE_DEBUGGABLE FInheritedType& OnGroupCreated(const UE::Flecs::Queries::FGroupByCreateGroupFunctionType& InCallback)
	{
		GetMutableGroupByCallbackDefinition().CreateGroupFunctionPtr = InCallback;
		return GetSelf();	
	}
	
	/** Sets the callback invoked when Flecs destroys a group. */
	FORCEINLINE_DEBUGGABLE FInheritedType& OnGroupDestroyed(const UE::Flecs::Queries::FGroupByDeleteGroupFunctionType& InCallback)
	{
		GetMutableGroupByCallbackDefinition().DeleteGroupFunctionPtr = InCallback;
		return GetSelf();	
	}

#pragma endregion GroupByFunctions
/** @} */
	
/**
 * @name Sources and traversal
 * @brief Selects term sources and relationship traversal expressions.
 * @{
 */
/**
 * @brief Sets the source of the current term.
 *
 * The overload family accepts a Flecs id, reflected struct or enum, C++ type
 * symbol, gameplay tag, or native gameplay tag. The source must be configured
 * after a term has been added or selected.
 *
 * @param InSource Entity or type that supplies the current term.
 * @return The derived builder for fluent chaining.
 */
	FORCEINLINE_DEBUGGABLE FInheritedType& Src(const FFlecsId InSource)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		this->GetQueryDefinition().Terms[LastTermIndex].Source.First.template InitializeAs<FFlecsQueryGeneratorInputType_FlecsId>();
		this->GetQueryDefinition().Terms[LastTermIndex].Source.First.template GetMutable<FFlecsQueryGeneratorInputType_FlecsId>().FlecsId = InSource;
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Src(const FString& InSource)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		this->GetQueryDefinition().Terms[LastTermIndex].Source.First.template InitializeAs<FFlecsQueryGeneratorInputType_String>();
		this->GetQueryDefinition().Terms[LastTermIndex].Source.First.template GetMutable<FFlecsQueryGeneratorInputType_String>().InputString = InSource;
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Src(const TSolidNotNull<const UScriptStruct*> InStruct)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		this->GetQueryDefinition().Terms[LastTermIndex].Source.First.template InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		this->GetQueryDefinition().Terms[LastTermIndex].Source.First.template GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = InStruct;
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Src(const TSolidNotNull<const UEnum*> InEnum)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		this->GetQueryDefinition().Terms[LastTermIndex].Source.First.template InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnum>();
		this->GetQueryDefinition().Terms[LastTermIndex].Source.First.template GetMutable<FFlecsQueryGeneratorInputType_ScriptEnum>().ScriptEnum = InEnum;
		return GetSelf();
	}
	
	/** Sets the current term source from an exact C++ type symbol. */
	FORCEINLINE_DEBUGGABLE FInheritedType& SrcCppType(const FString& InCppTypeName)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		this->GetQueryDefinition().Terms[LastTermIndex].Source.First.template InitializeAs<FFlecsQueryGeneratorInputType_CPPType>();
		this->GetQueryDefinition().Terms[LastTermIndex].Source.First.template GetMutable<FFlecsQueryGeneratorInputType_CPPType>().SymbolString = InCppTypeName;
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Src(const FSolidEnumSelector& InEnumSelector)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		this->GetQueryDefinition().Terms[LastTermIndex].Source.First.template InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnumConstant>();
		this->GetQueryDefinition().Terms[LastTermIndex].Source.First.template GetMutable<FFlecsQueryGeneratorInputType_ScriptEnumConstant>().EnumValue = InEnumSelector;
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Src(const FGameplayTag& InGameplayTag)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		this->GetQueryDefinition().Terms[LastTermIndex].Source.First.template InitializeAs<FFlecsQueryGeneratorInputType_GameplayTag>();
		this->GetQueryDefinition().Terms[LastTermIndex].Source.First.template GetMutable<FFlecsQueryGeneratorInputType_GameplayTag>().GameplayTag = InGameplayTag;
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Src(const FNativeGameplayTag& InNativeGameplayTag)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		this->GetQueryDefinition().Terms[LastTermIndex].Source.First.template InitializeAs<FFlecsQueryGeneratorInputType_GameplayTag>();
		this->GetQueryDefinition().Terms[LastTermIndex].Source.First.template GetMutable<FFlecsQueryGeneratorInputType_GameplayTag>().GameplayTag = InNativeGameplayTag.GetTag();
		return GetSelf();
	}
	
	template <typename T>
	FORCEINLINE_DEBUGGABLE FInheritedType& Src()
	{
		const std::string_view TypeName = nameof(T);
		return SrcCppType(FString(TypeName.data()));
	}
	
	/**
	 * @brief Adds an upward relationship traversal to the current term.
	 *
	 * @return The derived builder for fluent chaining.
	 * @see https://www.flecs.dev/flecs/Relationships.html#traversal
	 */
	FORCEINLINE_DEBUGGABLE FInheritedType& Up()
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		TInstancedStruct<FFlecsQueryExpression> UpExpr;
		UpExpr.InitializeAs<FFlecsQueryUpExpression>();
		
		this->GetQueryDefinition().Terms[LastTermIndex].Children.Add(UpExpr);
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Up(const FFlecsId InId)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		TInstancedStruct<FFlecsQueryExpression> UpExpr;
		UpExpr.InitializeAs<FFlecsQueryUpExpression>();
		UpExpr.GetMutable<FFlecsQueryUpExpression>().Traversal = FFlecsQueryGeneratorInput();
		UpExpr.GetMutable<FFlecsQueryUpExpression>().Traversal.GetValue().First.InitializeAs<FFlecsQueryGeneratorInputType_FlecsId>();
		UpExpr.GetMutable<FFlecsQueryUpExpression>().Traversal.GetValue().First
			.GetMutable<FFlecsQueryGeneratorInputType_FlecsId>().FlecsId = InId;
		
		this->GetQueryDefinition().Terms[LastTermIndex].Children.Add(UpExpr);
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Up(const UScriptStruct* InStruct)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		TInstancedStruct<FFlecsQueryExpression> UpExpr;
		UpExpr.InitializeAs<FFlecsQueryUpExpression>();
		UpExpr.GetMutable<FFlecsQueryUpExpression>().Traversal = FFlecsQueryGeneratorInput();
		UpExpr.GetMutable<FFlecsQueryUpExpression>().Traversal.GetValue().First.InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		UpExpr.GetMutable<FFlecsQueryUpExpression>().Traversal.GetValue().First
			.GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = InStruct;
		
		this->GetQueryDefinition().Terms[LastTermIndex].Children.Add(UpExpr);
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Up(const UEnum* InEnum)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		TInstancedStruct<FFlecsQueryExpression> UpExpr;
		UpExpr.InitializeAs<FFlecsQueryUpExpression>();
		UpExpr.GetMutable<FFlecsQueryUpExpression>().Traversal = FFlecsQueryGeneratorInput();
		UpExpr.GetMutable<FFlecsQueryUpExpression>().Traversal.GetValue().First.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnum>();
		UpExpr.GetMutable<FFlecsQueryUpExpression>().Traversal.GetValue().First
			.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnum>().ScriptEnum = InEnum;
		
		this->GetQueryDefinition().Terms[LastTermIndex].Children.Add(UpExpr);
		return GetSelf();
	}
	
	// Note: The InCppTypeName should be the exact match of what the EcsSymbol would be for the given C++ type.
	FORCEINLINE_DEBUGGABLE FInheritedType& Up(const FString& InCppTypeName)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		TInstancedStruct<FFlecsQueryExpression> UpExpr;
		UpExpr.InitializeAs<FFlecsQueryUpExpression>();
		UpExpr.GetMutable<FFlecsQueryUpExpression>().Traversal = FFlecsQueryGeneratorInput();
		UpExpr.GetMutable<FFlecsQueryUpExpression>().Traversal.GetValue().First.InitializeAs<FFlecsQueryGeneratorInputType_CPPType>();
		UpExpr.GetMutable<FFlecsQueryUpExpression>().Traversal.GetValue().First
			.GetMutable<FFlecsQueryGeneratorInputType_CPPType>().SymbolString = InCppTypeName;
		
		this->GetQueryDefinition().Terms[LastTermIndex].Children.Add(UpExpr);
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Up(const FSolidEnumSelector& InEnumSelector)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		TInstancedStruct<FFlecsQueryExpression> UpExpr;
		UpExpr.InitializeAs<FFlecsQueryUpExpression>();
		UpExpr.GetMutable<FFlecsQueryUpExpression>().Traversal = FFlecsQueryGeneratorInput();
		UpExpr.GetMutable<FFlecsQueryUpExpression>().Traversal.GetValue().First.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnumConstant>();
		UpExpr.GetMutable<FFlecsQueryUpExpression>().Traversal.GetValue().First
			.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnumConstant>().EnumValue = InEnumSelector;
		
		this->GetQueryDefinition().Terms[LastTermIndex].Children.Add(UpExpr);
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Up(const FGameplayTag& InGameplayTag)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		TInstancedStruct<FFlecsQueryExpression> UpExpr;
		UpExpr.InitializeAs<FFlecsQueryUpExpression>();
		UpExpr.GetMutable<FFlecsQueryUpExpression>().Traversal = FFlecsQueryGeneratorInput();
		UpExpr.GetMutable<FFlecsQueryUpExpression>().Traversal.GetValue().First.InitializeAs<FFlecsQueryGeneratorInputType_GameplayTag>();
		UpExpr.GetMutable<FFlecsQueryUpExpression>().Traversal.GetValue().First
			.GetMutable<FFlecsQueryGeneratorInputType_GameplayTag>().GameplayTag = InGameplayTag;
		
		this->GetQueryDefinition().Terms[LastTermIndex].Children.Add(UpExpr);
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Up(const FNativeGameplayTag& InNativeGameplayTag)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		TInstancedStruct<FFlecsQueryExpression> UpExpr;
		UpExpr.InitializeAs<FFlecsQueryUpExpression>();
		UpExpr.GetMutable<FFlecsQueryUpExpression>().Traversal = FFlecsQueryGeneratorInput();
		UpExpr.GetMutable<FFlecsQueryUpExpression>().Traversal.GetValue().First.InitializeAs<FFlecsQueryGeneratorInputType_GameplayTag>();
		UpExpr.GetMutable<FFlecsQueryUpExpression>().Traversal.GetValue().First
			.GetMutable<FFlecsQueryGeneratorInputType_GameplayTag>().GameplayTag = InNativeGameplayTag.GetTag();
		
		this->GetQueryDefinition().Terms[LastTermIndex].Children.Add(UpExpr);
		return GetSelf();
	}
	
	/** Adds an upward traversal using a C++ relationship type. */
	template <typename TTraversal>
	FORCEINLINE_DEBUGGABLE FInheritedType& Up()
	{
		if constexpr (Solid::IsScriptStruct<TTraversal>())
		{
			return Up(TBaseStructure<TTraversal>::Get());
		}
		else if constexpr (Solid::TStaticEnumConcept<TTraversal>)
		{
			return Up(StaticEnum<TTraversal>());
		}
		else
		{
			const std::string_view TypeName = nameof(TTraversal);
			return Up(FString(TypeName.data()));
		}
	}
	
	/**
	 * @brief Adds a cascading relationship traversal to the current term.
	 *
	 * @return The derived builder for fluent chaining.
	 * @see https://www.flecs.dev/flecs/Relationships.html#traversal
	 */
	FORCEINLINE_DEBUGGABLE FInheritedType& Cascade()
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		TInstancedStruct<FFlecsQueryExpression> CascadeExpr;
		CascadeExpr.InitializeAs<FFlecsQueryCascadeExpression>();
		
		this->GetQueryDefinition().Terms[LastTermIndex].Children.Add(CascadeExpr);
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Cascade(const FFlecsId InId)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		TInstancedStruct<FFlecsQueryExpression> CascadeExpr;
		CascadeExpr.InitializeAs<FFlecsQueryCascadeExpression>();
		CascadeExpr.GetMutable<FFlecsQueryCascadeExpression>().Traversal = FFlecsQueryGeneratorInput();
		CascadeExpr.GetMutable<FFlecsQueryCascadeExpression>().Traversal.GetValue().First.InitializeAs<FFlecsQueryGeneratorInputType_FlecsId>();
		CascadeExpr.GetMutable<FFlecsQueryCascadeExpression>().Traversal.GetValue().First.GetMutable<FFlecsQueryGeneratorInputType_FlecsId>().FlecsId = InId;
		
		this->GetQueryDefinition().Terms[LastTermIndex].Children.Add(CascadeExpr);
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Cascade(const UScriptStruct* InStruct)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		TInstancedStruct<FFlecsQueryExpression> CascadeExpr;
		CascadeExpr.InitializeAs<FFlecsQueryCascadeExpression>();
		CascadeExpr.GetMutable<FFlecsQueryCascadeExpression>().Traversal = FFlecsQueryGeneratorInput();
		CascadeExpr.GetMutable<FFlecsQueryCascadeExpression>().Traversal.GetValue().First.InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		CascadeExpr.GetMutable<FFlecsQueryCascadeExpression>().Traversal.GetValue().First.GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = InStruct;
		
		this->GetQueryDefinition().Terms[LastTermIndex].Children.Add(CascadeExpr);
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Cascade(const UEnum* InEnum)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		TInstancedStruct<FFlecsQueryExpression> CascadeExpr;
		CascadeExpr.InitializeAs<FFlecsQueryCascadeExpression>();
		CascadeExpr.GetMutable<FFlecsQueryCascadeExpression>().Traversal = FFlecsQueryGeneratorInput();
		CascadeExpr.GetMutable<FFlecsQueryCascadeExpression>().Traversal.GetValue().First.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnum>();
		CascadeExpr.GetMutable<FFlecsQueryCascadeExpression>().Traversal.GetValue().First.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnum>().ScriptEnum = InEnum;
		
		this->GetQueryDefinition().Terms[LastTermIndex].Children.Add(CascadeExpr);
		return GetSelf();
	}
	
	// Note: The InCppTypeName should be the exact match of what the EcsSymbol would be for the given C++ type.
	FORCEINLINE_DEBUGGABLE FInheritedType& Cascade(const FString& InCppTypeName)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		TInstancedStruct<FFlecsQueryExpression> CascadeExpr;
		CascadeExpr.InitializeAs<FFlecsQueryCascadeExpression>();
		CascadeExpr.GetMutable<FFlecsQueryCascadeExpression>().Traversal = FFlecsQueryGeneratorInput();
		CascadeExpr.GetMutable<FFlecsQueryCascadeExpression>().Traversal.GetValue().First.InitializeAs<FFlecsQueryGeneratorInputType_CPPType>();
		CascadeExpr.GetMutable<FFlecsQueryCascadeExpression>().Traversal.GetValue().First.GetMutable<FFlecsQueryGeneratorInputType_CPPType>().SymbolString = InCppTypeName;
		
		this->GetQueryDefinition().Terms[LastTermIndex].Children.Add(CascadeExpr);
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Cascade(const FSolidEnumSelector& InEnumSelector)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		TInstancedStruct<FFlecsQueryExpression> CascadeExpr;
		CascadeExpr.InitializeAs<FFlecsQueryCascadeExpression>();
		CascadeExpr.GetMutable<FFlecsQueryCascadeExpression>().Traversal = FFlecsQueryGeneratorInput();
		CascadeExpr.GetMutable<FFlecsQueryCascadeExpression>().Traversal.GetValue().First.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnumConstant>();
		CascadeExpr.GetMutable<FFlecsQueryCascadeExpression>().Traversal.GetValue().First.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnumConstant>().EnumValue = InEnumSelector;
		
		this->GetQueryDefinition().Terms[LastTermIndex].Children.Add(CascadeExpr);
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Cascade(const FGameplayTag& InGameplayTag)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		TInstancedStruct<FFlecsQueryExpression> CascadeExpr;
		CascadeExpr.InitializeAs<FFlecsQueryCascadeExpression>();
		CascadeExpr.GetMutable<FFlecsQueryCascadeExpression>().Traversal = FFlecsQueryGeneratorInput();
		CascadeExpr.GetMutable<FFlecsQueryCascadeExpression>().Traversal.GetValue().First.InitializeAs<FFlecsQueryGeneratorInputType_GameplayTag>();
		CascadeExpr.GetMutable<FFlecsQueryCascadeExpression>().Traversal.GetValue().First.GetMutable<FFlecsQueryGeneratorInputType_GameplayTag>().GameplayTag = InGameplayTag;
		
		this->GetQueryDefinition().Terms[LastTermIndex].Children.Add(CascadeExpr);
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Cascade(const FNativeGameplayTag& InNativeGameplayTag)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		TInstancedStruct<FFlecsQueryExpression> CascadeExpr;
		CascadeExpr.InitializeAs<FFlecsQueryCascadeExpression>();
		CascadeExpr.GetMutable<FFlecsQueryCascadeExpression>().Traversal = FFlecsQueryGeneratorInput();
		CascadeExpr.GetMutable<FFlecsQueryCascadeExpression>().Traversal.GetValue().First.InitializeAs<FFlecsQueryGeneratorInputType_GameplayTag>();
		CascadeExpr.GetMutable<FFlecsQueryCascadeExpression>().Traversal.GetValue().First.GetMutable<FFlecsQueryGeneratorInputType_GameplayTag>().GameplayTag = InNativeGameplayTag.GetTag();
		
		this->GetQueryDefinition().Terms[LastTermIndex].Children.Add(CascadeExpr);
		return GetSelf();
	}
	
	/** Adds a cascading traversal using a C++ relationship type. */
	template <typename TTraversal>
	FORCEINLINE_DEBUGGABLE FInheritedType& Cascade()
	{
		if constexpr (Solid::IsScriptStruct<TTraversal>())
		{
			return Cascade(TBaseStructure<TTraversal>::Get());
		}
		else if constexpr (Solid::TStaticEnumConcept<TTraversal>)
		{
			return Cascade(StaticEnum<TTraversal>());
		}
		else
		{
			const std::string_view TypeName = nameof(TTraversal);
			return Cascade(FString(TypeName.data()));
		}
	}
	
	/** Adds descending traversal to the current term. */
	FORCEINLINE_DEBUGGABLE FInheritedType& Desc()
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		
		TInstancedStruct<FFlecsQueryExpression> DescExpr;
		DescExpr.InitializeAs<FFlecsQueryDescendingExpression>();
		
		this->GetQueryDefinition().Terms[LastTermIndex].Children.Add(DescExpr);
		return GetSelf();
	}
	
	
	/**
	 * @brief Adds a Flecs Script expression as a child of the current term.
	 *
	 * @param InExpression Script expression to append.
	 * @return The derived builder for fluent chaining.
	 */
	FORCEINLINE_DEBUGGABLE FInheritedType& ScriptExpr(const FFlecsQueryScriptExpr& InExpression)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		
		TInstancedStruct<FFlecsQueryExpression> ScriptExpr;
		ScriptExpr.InitializeAs<FFlecsQueryScriptExpression>();
		ScriptExpr.GetMutable<FFlecsQueryScriptExpression>().ScriptExpr = InExpression;
		
		this->GetQueryDefinition().Terms[LastTermIndex].Children.Add(ScriptExpr);
		return GetSelf();
	}
	
	/**
	 * @brief Mutates the complete current term expression.
	 *
	 * @param InModifier Function that edits the selected term.
	 * @return The derived builder for fluent chaining.
	 */
	FORCEINLINE_DEBUGGABLE FInheritedType& ModifyLastTerm(const TFunctionRef<void(FFlecsQueryTermExpression&)>& InModifier)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		InModifier(this->GetQueryDefinition().Terms[LastTermIndex]);
		return GetSelf();
	}
	
	/**
	 * @brief Adds a custom query expression to the definition.
	 *
	 * @tparam TExpression Type derived from FFlecsQueryExpression.
	 * @param InExpression Expression to append.
	 * @return The derived builder for fluent chaining.
	 */
	template <UE::Flecs::Queries::TQueryExpressionConcept TExpression>
	FORCEINLINE_DEBUGGABLE FInheritedType& AddExpression(const TExpression& InExpression)
	{
		this->GetQueryDefinition().AddAdditionalExpression(InExpression);
		return GetSelf();
	}
/** @} */
}; // struct TFlecsQueryBuilderBase
