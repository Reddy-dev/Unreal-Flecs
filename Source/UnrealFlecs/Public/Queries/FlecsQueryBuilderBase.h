// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsQueryFlags.h"
#include "Enums/FlecsQueryCache.h"
#include "Enums/FlecsQueryInOut.h"
#include "Expressions/FlecsQueryTermExpression.h"
#include "Generator/FlecsQueryGeneratorInputType.h"
#include "FlecsQueryDefinition.h"
#include "Callbacks/FlecsOrderByCallbackDefinition.h"
#include "Expressions/FlecsQueryCascadeExpression.h"
#include "Expressions/FlecsQueryGroupByExpression.h"
#include "Expressions/FlecsQueryOrderByExpression.h"
#include "Expressions/FlecsQueryUpExpression.h"

namespace Unreal::Flecs::Queries
{
	template <typename T>
	concept CQueryDefinitionRecordInputType = std::is_convertible<T, FFlecsId>::value
		|| std::is_convertible<T, const UScriptStruct*>::value
		|| std::is_convertible<T, FString>::value
		|| std::is_convertible<T, const UEnum*>::value
		|| std::is_convertible<T, FSolidEnumSelector>::value;
	
} // namespace Unreal::Flecs::Queries

template <typename TInherited>
struct TFlecsQueryBuilderBase
{
	using FInheritedType = TInherited;
	
private:
	
	FORCEINLINE_DEBUGGABLE FInheritedType& GetSelf()
	{
		return static_cast<TInherited&>(*this);
	}
	
	FORCEINLINE_DEBUGGABLE const FInheritedType& GetSelf() const
	{
		return static_cast<const TInherited&>(*this);
	}
	
public:
	
	FORCEINLINE_DEBUGGABLE FFlecsQueryDefinition& GetQueryDefinition() const
	{
		return this->GetSelf().GetQueryDefinition_Impl();
	}
	
	mutable int32 LastTermIndex = -1;
	
	FORCEINLINE_DEBUGGABLE FInheritedType& AddTerm(const FFlecsQueryTermExpression& InTerm)
	{
		this->GetQueryDefinition().AddQueryTerm(InTerm);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& TermAt(const int32 InTermIndex)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		LastTermIndex = InTermIndex;
		return GetSelf();
	}
	
	/*FORCEINLINE_DEBUGGABLE FInheritedType& TermAt(*/
	
#pragma region QueryDefinitionProperties
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Cache(const EFlecsQueryCacheType InCacheType = EFlecsQueryCacheType::Default)
	{
		this->GetQueryDefinition().CacheType = InCacheType;
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& DetectChanges(const bool bInDetectChanges = true)
	{
		this->GetQueryDefinition().bDetectChanges = bInDetectChanges;
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Flags(const uint8 InFlags)
	{
		this->GetQueryDefinition().Flags = InFlags;
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Flags(const EFlecsQueryFlags InFlags)
	{
		this->GetQueryDefinition().Flags = static_cast<uint8>(InFlags);
		return GetSelf();
	}
	
#pragma endregion QueryDefinitionProperties
	
#pragma region TermOperatorExpressions
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Oper(const EFlecsQueryOperator InOperator)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		
		this->GetQueryDefinition().Terms[LastTermIndex].Operator = InOperator;
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& And()
	{
		return Oper(EFlecsQueryOperator::And);
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Or()
	{
		return Oper(EFlecsQueryOperator::Or);
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Not()
	{
		return Oper(EFlecsQueryOperator::Not);
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Optional()
	{
		return Oper(EFlecsQueryOperator::Optional);
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& AndFrom()
	{
		return Oper(EFlecsQueryOperator::AndFrom);
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& OrFrom()
	{
		return Oper(EFlecsQueryOperator::OrFrom);
	}
	
#pragma endregion TermOperatorExpressions
	
#pragma region ReadWriteInOutExpressions
	
	FORCEINLINE_DEBUGGABLE FInheritedType& InOutExpression(const EFlecsQueryInOut InInOut, const bool bStage = false)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		
		this->GetQueryDefinition().Terms[LastTermIndex].InOut = InInOut;
		this->GetQueryDefinition().Terms[LastTermIndex].bStage = bStage;
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& In()
	{
		return InOutExpression(EFlecsQueryInOut::Read, false);
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Out()
	{
		return InOutExpression(EFlecsQueryInOut::Write, false);
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& InOut()
	{
		return InOutExpression(EFlecsQueryInOut::ReadWrite, false);
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Read()
	{
		return InOutExpression(EFlecsQueryInOut::Read, true);
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Write()
	{
		return InOutExpression(EFlecsQueryInOut::Write, true);
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& ReadWrite()
	{
		return InOutExpression(EFlecsQueryInOut::ReadWrite, true);
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Filter()
	{
		return InOutExpression(EFlecsQueryInOut::Filter, false);
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& InOutNone()
	{
		return InOutExpression(EFlecsQueryInOut::None, false);
	}
	
#pragma endregion ReadWriteInOutExpressions
	
#pragma region TermHelperFunctions
	
private:
	FORCEINLINE_DEBUGGABLE FInheritedType& WithCppType_Internal(const std::string_view TypeName)
	{
		const FString TypeNameFString = FString(TypeName.data());
		
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_CPPType>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_CPPType>().SymbolString = TypeNameFString;
		
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
		
		FFlecsQueryTermExpression SecondExpr;
		SecondExpr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_CPPType>();
		SecondExpr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_CPPType>().SymbolString = TypeNameFString;
		
		FFlecsQueryTermExpression& TermExpr = this->GetQueryDefinition().Terms[LastTermIndex];
		
		const FFlecsQueryTerm FirstTerm = TermExpr.Term;
		
		TermExpr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_Pair>();
		TermExpr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_Pair>().First = FirstTerm.InputType;
		TermExpr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_Pair>().Second = SecondExpr.Term.InputType;
		return GetSelf();
	}
	
public:
	
	FORCEINLINE_DEBUGGABLE FInheritedType& With(const FFlecsId InId)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_FlecsId>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_FlecsId>().FlecsId = InId;
		
		this->GetQueryDefinition().AddQueryTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& With(const TSolidNotNull<const UScriptStruct*> InStruct)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = InStruct;
		
		this->AddTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& With(const FString& InString)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_String>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_String>().InputString = InString;
		
		this->GetQueryDefinition().AddQueryTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& With(const TSolidNotNull<const UEnum*> InEnum)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnum>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnum>().ScriptEnum = InEnum;
		
		this->GetQueryDefinition().AddQueryTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& With(const FSolidEnumSelector& InEnumSelector)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnumConstant>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnumConstant>().EnumValue = InEnumSelector;
		
		this->GetQueryDefinition().AddQueryTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		return GetSelf();
	}
	
	template <typename T>
	FORCEINLINE_DEBUGGABLE FInheritedType& With()
	{
		if constexpr (Solid::IsScriptStruct<T>())
		{
			this->With(TBaseStructure<T>::Get());
		}
		else if constexpr (Solid::TStaticEnumConcept<T>)
		{
			this->With(StaticEnum<T>());
		}
		else
		{
			const std::string_view TypeName = nameof(T);
			this->WithCppType_Internal(TypeName);
		}
		
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Without(const FFlecsId InId)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_FlecsId>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_FlecsId>().FlecsId = InId;
		
		this->AddTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		this->Not();
		
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Without(const TSolidNotNull<const UScriptStruct*> InStruct)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = InStruct;
		
		this->AddTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		this->Not();
		
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Without(const FString& InString)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_String>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_String>().InputString = InString;
		
		this->AddTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		this->Not();
		
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Without(const TSolidNotNull<const UEnum*> InEnum)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnum>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnum>().ScriptEnum = InEnum;
		
		this->AddTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		this->Not();
		
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Without(const FSolidEnumSelector& InEnumSelector)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnumConstant>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnumConstant>().EnumValue = InEnumSelector;
		
		this->AddTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		this->Not();
		
		return GetSelf();
	}
	
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
		else
		{
			const std::string_view TypeName = nameof(T);
			this->WithoutCppType_Internal(TypeName);
		}
		
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Second(const FFlecsId InId)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		
		FFlecsQueryTermExpression SecondExpr;
		SecondExpr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_FlecsId>();
		SecondExpr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_FlecsId>().FlecsId = InId;
		
		FFlecsQueryTermExpression& TermExpr = this->GetQueryDefinition().Terms[LastTermIndex];
		
		const FFlecsQueryTerm FirstTerm = TermExpr.Term;
		
		TermExpr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_Pair>();
		TermExpr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_Pair>().First = FirstTerm.InputType;
		TermExpr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_Pair>().Second = SecondExpr.Term.InputType;
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Second(const TSolidNotNull<const UScriptStruct*> InStruct)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		
		FFlecsQueryTermExpression SecondExpr;
		SecondExpr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		SecondExpr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = InStruct;
		
		FFlecsQueryTermExpression& TermExpr = this->GetQueryDefinition().Terms[LastTermIndex];
		
		const FFlecsQueryTerm FirstTerm = TermExpr.Term;
		
		TermExpr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_Pair>();
		TermExpr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_Pair>().First = FirstTerm.InputType;
		TermExpr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_Pair>().Second = SecondExpr.Term.InputType;
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Second(const FString& InString)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		
		FFlecsQueryTermExpression SecondExpr;
		SecondExpr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_String>();
		SecondExpr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_String>().InputString = InString;
		
		FFlecsQueryTermExpression& TermExpr = this->GetQueryDefinition().Terms[LastTermIndex];
		
		const FFlecsQueryTerm FirstTerm = TermExpr.Term;
		
		TermExpr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_Pair>();
		TermExpr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_Pair>().First = FirstTerm.InputType;
		TermExpr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_Pair>().Second = SecondExpr.Term.InputType;
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Second(const TSolidNotNull<const UEnum*> InEnum)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		
		FFlecsQueryTermExpression SecondExpr;
		SecondExpr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnum>();
		SecondExpr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnum>().ScriptEnum = InEnum;
		
		FFlecsQueryTermExpression& TermExpr = this->GetQueryDefinition().Terms[LastTermIndex];
		
		const FFlecsQueryTerm FirstTerm = TermExpr.Term;
		
		TermExpr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_Pair>();
		TermExpr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_Pair>().First = FirstTerm.InputType;
		TermExpr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_Pair>().Second = SecondExpr.Term.InputType;
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Second(const FSolidEnumSelector& InEnumSelector)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		
		FFlecsQueryTermExpression SecondExpr;
		SecondExpr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnumConstant>();
		SecondExpr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnumConstant>().EnumValue = InEnumSelector;
		
		FFlecsQueryTermExpression& TermExpr = this->GetQueryDefinition().Terms[LastTermIndex];
		const FFlecsQueryTerm FirstTerm = TermExpr.Term;
		TermExpr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_Pair>();
		TermExpr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_Pair>().First = FirstTerm.InputType;
		TermExpr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_Pair>().Second = SecondExpr.Term.InputType;
		return GetSelf();
	}
	
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
		else
		{
			const std::string_view TypeName = nameof(T);
			const FString TypeNameFString = FString(TypeName.data());
			this->Second(TypeNameFString);
		}
		
		return GetSelf();
	}
	
	template <Unreal::Flecs::Queries::CQueryDefinitionRecordInputType TFirst, Unreal::Flecs::Queries::CQueryDefinitionRecordInputType TSecond>
	FORCEINLINE_DEBUGGABLE FInheritedType& WithPair(const TFirst& InFirst, const TSecond& InSecond)
	{
		this->With(InFirst);
		this->Second(InSecond);
		return GetSelf();
	}
	
	template <Unreal::Flecs::Queries::CQueryDefinitionRecordInputType TFirst, Unreal::Flecs::Queries::CQueryDefinitionRecordInputType TSecond>
	FORCEINLINE_DEBUGGABLE FInheritedType& WithoutPair(const TFirst& InFirst, const TSecond& InSecond)
	{
		this->Without(InFirst);
		this->Second(InSecond);
		return GetSelf();
	}
	
	template <typename T, Unreal::Flecs::Queries::CQueryDefinitionRecordInputType TSecond>
	FORCEINLINE_DEBUGGABLE FInheritedType& WithPair(const TSecond& InSecond)
	{
		this->With<T>();
		this->Second(InSecond);
		return GetSelf();
	}
	
	template <typename T, Unreal::Flecs::Queries::CQueryDefinitionRecordInputType TSecond>
	FORCEINLINE_DEBUGGABLE FInheritedType& WithoutPair(const TSecond& InSecond)
	{
		this->Without<T>();
		this->Second(InSecond);
		return GetSelf();
	}
	
	template <typename T, Unreal::Flecs::Queries::CQueryDefinitionRecordInputType TFirst>
	FORCEINLINE_DEBUGGABLE FInheritedType& WithPairSecond(const TFirst& InFirst)
	{
		this->With(InFirst);
		this->Second<T>();
		return GetSelf();
	}
	
	template <typename T, Unreal::Flecs::Queries::CQueryDefinitionRecordInputType TFirst>
	FORCEINLINE_DEBUGGABLE FInheritedType& WithoutPairSecond(const TFirst& InFirst)
	{
		this->Without(InFirst);
		this->Second<T>();
		return GetSelf();
	}
	
	template <typename TFirst, typename TSecond>
	FORCEINLINE_DEBUGGABLE FInheritedType& WithPair()
	{
		this->With<TFirst>();
		this->Second<TSecond>();
		return GetSelf();
	}
	
	template <typename TFirst, typename TSecond>
	FORCEINLINE_DEBUGGABLE FInheritedType& WithoutPair()
	{
		this->Without<TFirst>();
		this->Second<TSecond>();
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& WithPair(const FSolidEnumSelector& InPair)
	{
		WithPair(InPair.Class, InPair.Value);
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& WithoutPair(const FSolidEnumSelector& InPair)
	{
		WithoutPair(InPair.Class, InPair.Value);
		return GetSelf();
	}
	
#pragma endregion TermHelperFunctions
	
#pragma region OrderByFunctions
	
	FORCEINLINE_DEBUGGABLE FInheritedType& OrderBy(const FFlecsId InId, Unreal::Flecs::Queries::FOrderByFunctionType InFunction)
	{
		TInstancedStruct<FFlecsQueryExpression> OrderByExpr;
		OrderByExpr.InitializeAs<FFlecsQueryOrderByCPPExpressionWrapper>();
		OrderByExpr.GetMutable<FFlecsQueryOrderByCPPExpressionWrapper>().OrderByInput.InitializeAs<FFlecsQueryGeneratorInputType_FlecsId>();
		OrderByExpr.GetMutable<FFlecsQueryOrderByCPPExpressionWrapper>().OrderByInput.GetMutable<FFlecsQueryGeneratorInputType_FlecsId>().FlecsId = InId;
		OrderByExpr.GetMutable<FFlecsQueryOrderByCPPExpressionWrapper>().OrderByFunction = InFunction;
		
		this->GetQueryDefinition().OtherExpressions.Add(OrderByExpr);
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& OrderBy(const UScriptStruct* InStruct, Unreal::Flecs::Queries::FOrderByFunctionType InFunction)
	{
		TInstancedStruct<FFlecsQueryExpression> OrderByExpr;
		OrderByExpr.InitializeAs<FFlecsQueryOrderByCPPExpressionWrapper>();
		OrderByExpr.GetMutable<FFlecsQueryOrderByCPPExpressionWrapper>().OrderByInput.InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		OrderByExpr.GetMutable<FFlecsQueryOrderByCPPExpressionWrapper>().OrderByInput.GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = InStruct;
		OrderByExpr.GetMutable<FFlecsQueryOrderByCPPExpressionWrapper>().OrderByFunction = InFunction;
		
		this->GetQueryDefinition().OtherExpressions.Add(OrderByExpr);
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& OrderBy(const UEnum* InEnum, const Unreal::Flecs::Queries::FOrderByFunctionType& InFunction)
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
	FORCEINLINE_DEBUGGABLE FInheritedType& OrderBy(const FString& InCppTypeName, const Unreal::Flecs::Queries::FOrderByFunctionType& InFunction)
	{
		TInstancedStruct<FFlecsQueryExpression> OrderByExpr;
		OrderByExpr.InitializeAs<FFlecsQueryOrderByCPPExpressionWrapper>();
		OrderByExpr.GetMutable<FFlecsQueryOrderByCPPExpressionWrapper>().OrderByInput.InitializeAs<FFlecsQueryGeneratorInputType_CPPType>();
		OrderByExpr.GetMutable<FFlecsQueryOrderByCPPExpressionWrapper>().OrderByInput.GetMutable<FFlecsQueryGeneratorInputType_CPPType>().SymbolString = InCppTypeName;
		OrderByExpr.GetMutable<FFlecsQueryOrderByCPPExpressionWrapper>().OrderByFunction = InFunction;
		
		this->GetQueryDefinition().OtherExpressions.Add(OrderByExpr);
		return GetSelf();
	}
	
	template <typename T>
	FORCEINLINE_DEBUGGABLE FInheritedType& OrderBy(const Unreal::Flecs::Queries::TOrderByFunction<T>& InFunction)
	{
		const std::string_view TypeName = nameof(T);
		
		TInstancedStruct<FFlecsQueryExpression> OrderByExpr;
		OrderByExpr.InitializeAs<FFlecsQueryOrderByCPPExpressionWrapper>();
		OrderByExpr.GetMutable<FFlecsQueryOrderByCPPExpressionWrapper>().OrderByInput.InitializeAs<FFlecsQueryGeneratorInputType_CPPType>();
		OrderByExpr.GetMutable<FFlecsQueryOrderByCPPExpressionWrapper>().OrderByInput.GetMutable<FFlecsQueryGeneratorInputType_CPPType>().SymbolString = FString(TypeName.data());
		OrderByExpr.GetMutable<FFlecsQueryOrderByCPPExpressionWrapper>().OrderByFunction = Unreal::Flecs::Queries::ConvertToUntypedOrderByFunction(InFunction);
		
		this->GetQueryDefinition().OtherExpressions.Add(OrderByExpr);
		return GetSelf();
	}
	
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
	
	template <typename T>
	FORCEINLINE_DEBUGGABLE FInheritedType& OrderByCallbackDefinition(const TInstancedStruct<FFlecsOrderByCallbackDefinition>& InCallbackDefinition)
	{
		const std::string_view TypeName = nameof(T);
		
		TInstancedStruct<FFlecsQueryExpression> OrderByExpr;
		OrderByExpr.InitializeAs<FFlecsQueryOrderByExpression>();
		OrderByExpr.GetMutable<FFlecsQueryOrderByExpression>().OrderByInput.InitializeAs<FFlecsQueryGeneratorInputType_CPPType>();
		OrderByExpr.GetMutable<FFlecsQueryOrderByExpression>().OrderByInput.GetMutable<FFlecsQueryGeneratorInputType_CPPType>().SymbolString = FString(TypeName.data());
		OrderByExpr.GetMutable<FFlecsQueryOrderByExpression>().OrderByCallback = InCallbackDefinition;
		
		this->GetQueryDefinition().OtherExpressions.Add(OrderByExpr);
		return GetSelf();
	}
	
#pragma endregion OrderByFunctions
	
#pragma region GroupByFunctions
	
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
	
	template <typename T>
	FORCEINLINE_DEBUGGABLE FInheritedType& GroupBy()
	{
		const std::string_view TypeName = nameof(T);
		return GroupBy(FString(TypeName.data()));
	}

#pragma endregion GroupByFunctions
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Src(const FString& InSource)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		this->GetQueryDefinition().Terms[LastTermIndex].Source.template InitializeAs<FFlecsQueryGeneratorInputType_String>();
		this->GetQueryDefinition().Terms[LastTermIndex].Source.template GetMutable<FFlecsQueryGeneratorInputType_String>().InputString = InSource;
		return GetSelf();
	}
	
	/*FORCEINLINE_DEBUGGABLE FInheritedType& Src(const TSolidNotNull<const UScriptStruct*> InStruct)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		this->GetQueryDefinition().Terms[LastTermIndex].Source.template InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		this->GetQueryDefinition().Terms[LastTermIndex].Source.template GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = InStruct;
		return GetSelf();
	}*/
	
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
		UpExpr.GetMutable<FFlecsQueryUpExpression>().Traversal = TInstancedStruct<FFlecsQueryGeneratorInputType_FlecsId>::Make();
		UpExpr.GetMutable<FFlecsQueryUpExpression>().Traversal.GetValue().GetMutable<FFlecsQueryGeneratorInputType_FlecsId>().FlecsId = InId;
		
		this->GetQueryDefinition().Terms[LastTermIndex].Children.Add(UpExpr);
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Up(const UScriptStruct* InStruct)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		TInstancedStruct<FFlecsQueryExpression> UpExpr;
		UpExpr.InitializeAs<FFlecsQueryUpExpression>();
		UpExpr.GetMutable<FFlecsQueryUpExpression>().Traversal = TInstancedStruct<FFlecsQueryGeneratorInputType_ScriptStruct>::Make();
		UpExpr.GetMutable<FFlecsQueryUpExpression>().Traversal.GetValue().GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = InStruct;
		
		this->GetQueryDefinition().Terms[LastTermIndex].Children.Add(UpExpr);
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Up(const UEnum* InEnum)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		TInstancedStruct<FFlecsQueryExpression> UpExpr;
		UpExpr.InitializeAs<FFlecsQueryUpExpression>();
		UpExpr.GetMutable<FFlecsQueryUpExpression>().Traversal = TInstancedStruct<FFlecsQueryGeneratorInputType_ScriptEnum>::Make();
		UpExpr.GetMutable<FFlecsQueryUpExpression>().Traversal.GetValue().GetMutable<FFlecsQueryGeneratorInputType_ScriptEnum>().ScriptEnum = InEnum;
		
		this->GetQueryDefinition().Terms[LastTermIndex].Children.Add(UpExpr);
		return GetSelf();
	}
	
	// Note: The InCppTypeName should be the exact match of what the EcsSymbol would be for the given C++ type.
	FORCEINLINE_DEBUGGABLE FInheritedType& Up(const FString& InCppTypeName)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		TInstancedStruct<FFlecsQueryExpression> UpExpr;
		UpExpr.InitializeAs<FFlecsQueryUpExpression>();
		UpExpr.GetMutable<FFlecsQueryUpExpression>().Traversal = TInstancedStruct<FFlecsQueryGeneratorInputType_CPPType>::Make();
		UpExpr.GetMutable<FFlecsQueryUpExpression>().Traversal.GetValue().GetMutable<FFlecsQueryGeneratorInputType_CPPType>().SymbolString = InCppTypeName;
		
		this->GetQueryDefinition().Terms[LastTermIndex].Children.Add(UpExpr);
		return GetSelf();
	}
	
	template <typename TTraversal>
	FORCEINLINE_DEBUGGABLE FInheritedType& Up()
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		const std::string_view TypeName = nameof(TTraversal);
		
		TInstancedStruct<FFlecsQueryExpression> UpExpr;
		UpExpr.InitializeAs<FFlecsQueryUpExpression>();
		UpExpr.GetMutable<FFlecsQueryUpExpression>().Traversal = TInstancedStruct<FFlecsQueryGeneratorInputType_CPPType>::Make();
		UpExpr.GetMutable<FFlecsQueryUpExpression>().Traversal.GetValue().GetMutable<FFlecsQueryGeneratorInputType_CPPType>().SymbolString = FString(TypeName.data());
		
		this->GetQueryDefinition().Terms[LastTermIndex].Children.Add(UpExpr);
		return GetSelf();
	}
	
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
		CascadeExpr.GetMutable<FFlecsQueryCascadeExpression>().Traversal = TInstancedStruct<FFlecsQueryGeneratorInputType_FlecsId>::Make();
		CascadeExpr.GetMutable<FFlecsQueryCascadeExpression>().Traversal.GetValue().GetMutable<FFlecsQueryGeneratorInputType_FlecsId>().FlecsId = InId;
		
		this->GetQueryDefinition().Terms[LastTermIndex].Children.Add(CascadeExpr);
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Cascade(const UScriptStruct* InStruct)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		TInstancedStruct<FFlecsQueryExpression> CascadeExpr;
		CascadeExpr.InitializeAs<FFlecsQueryCascadeExpression>();
		CascadeExpr.GetMutable<FFlecsQueryCascadeExpression>().Traversal = TInstancedStruct<FFlecsQueryGeneratorInputType_ScriptStruct>::Make();
		CascadeExpr.GetMutable<FFlecsQueryCascadeExpression>().Traversal.GetValue().GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = InStruct;
		
		this->GetQueryDefinition().Terms[LastTermIndex].Children.Add(CascadeExpr);
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Cascade(const UEnum* InEnum)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		TInstancedStruct<FFlecsQueryExpression> CascadeExpr;
		CascadeExpr.InitializeAs<FFlecsQueryCascadeExpression>();
		CascadeExpr.GetMutable<FFlecsQueryCascadeExpression>().Traversal = TInstancedStruct<FFlecsQueryGeneratorInputType_ScriptEnum>::Make();
		CascadeExpr.GetMutable<FFlecsQueryCascadeExpression>().Traversal.GetValue().GetMutable<FFlecsQueryGeneratorInputType_ScriptEnum>().ScriptEnum = InEnum;
		
		this->GetQueryDefinition().Terms[LastTermIndex].Children.Add(CascadeExpr);
		return GetSelf();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Cascade(const FString& InCppTypeName)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		TInstancedStruct<FFlecsQueryExpression> CascadeExpr;
		CascadeExpr.InitializeAs<FFlecsQueryCascadeExpression>();
		CascadeExpr.GetMutable<FFlecsQueryCascadeExpression>().Traversal = TInstancedStruct<FFlecsQueryGeneratorInputType_CPPType>::Make();
		CascadeExpr.GetMutable<FFlecsQueryCascadeExpression>().Traversal.GetValue().GetMutable<FFlecsQueryGeneratorInputType_CPPType>().SymbolString = InCppTypeName;
		
		this->GetQueryDefinition().Terms[LastTermIndex].Children.Add(CascadeExpr);
		return GetSelf();
	}
	
	template <typename TTraversal>
	FORCEINLINE_DEBUGGABLE FInheritedType& Cascade()
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		const std::string_view TypeName = nameof(TTraversal);
		TInstancedStruct<FFlecsQueryExpression> CascadeExpr;
		CascadeExpr.InitializeAs<FFlecsQueryCascadeExpression>();
		CascadeExpr.GetMutable<FFlecsQueryCascadeExpression>().Traversal = TInstancedStruct<FFlecsQueryGeneratorInputType_CPPType>::Make();
		CascadeExpr.GetMutable<FFlecsQueryCascadeExpression>().Traversal.GetValue().GetMutable<FFlecsQueryGeneratorInputType_CPPType>().SymbolString = FString(TypeName.data());
		
		this->GetQueryDefinition().Terms[LastTermIndex].Children.Add(CascadeExpr);
		return GetSelf();
	}
	
	
	FORCEINLINE_DEBUGGABLE FInheritedType& ModifyLastTerm(const TFunctionRef<void(FFlecsQueryTermExpression&)>& InModifier)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		InModifier(this->GetQueryDefinition().Terms[LastTermIndex]);
		return GetSelf();
	}
	
	template <Unreal::Flecs::Queries::TQueryExpressionConcept TExpression>
	FORCEINLINE_DEBUGGABLE FInheritedType& AddExpression(const TExpression& InExpression)
	{
		this->GetQueryDefinition().AddAdditionalExpression(InExpression);
		return GetSelf();
	}
	
}; // struct TFlecsQueryBuilderBase
