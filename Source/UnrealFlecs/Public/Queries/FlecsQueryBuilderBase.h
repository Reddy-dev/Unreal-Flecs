// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsQueryFlags.h"
#include "Enums/FlecsQueryCache.h"
#include "Enums/FlecsQueryInOut.h"
#include "Expressions/FlecsQueryTermExpression.h"
#include "Generator/FlecsQueryGeneratorInputType.h"
#include "FlecsQueryDefinition.h"

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
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Get()
	{
		return static_cast<TInherited&>(*this);
	}
	
	FORCEINLINE_DEBUGGABLE const FInheritedType& Get() const
	{
		return static_cast<const TInherited&>(*this);
	}
	
	FORCEINLINE_DEBUGGABLE FFlecsQueryDefinition& GetQueryDefinition() const
	{
		return this->Get().GetQueryDefinition_Impl();
	}
	
public:
	mutable int32 LastTermIndex = -1;
	
	FORCEINLINE_DEBUGGABLE FInheritedType& AddTerm(const FFlecsQueryTermExpression& InTerm)
	{
		this->GetQueryDefinition().AddQueryTerm(InTerm);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		return Get();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& TermAt(const int32 InTermIndex)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		LastTermIndex = InTermIndex;
		return Get();
	}
	
#pragma region QueryDefinitionProperties
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Cache(const EFlecsQueryCacheType InCacheType = EFlecsQueryCacheType::Default)
	{
		this->GetQueryDefinition().CacheType = InCacheType;
		return Get();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& DetectChanges(const bool bInDetectChanges = true)
	{
		this->GetQueryDefinition().bDetectChanges = bInDetectChanges;
		return Get();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Flags(const uint8 InFlags)
	{
		this->GetQueryDefinition().Flags = InFlags;
		return Get();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Flags(const EFlecsQueryFlags InFlags)
	{
		this->GetQueryDefinition().Flags = static_cast<uint8>(InFlags);
		return Get();
	}
	
#pragma endregion QueryDefinitionProperties
	
#pragma region TermOperatorExpressions
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Oper(const EFlecsQueryOperator InOperator)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		
		this->GetQueryDefinition().Terms[LastTermIndex].Operator = InOperator;
		return Get();
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
		return Get();
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
	
	FORCEINLINE_DEBUGGABLE FInheritedType& With(const FFlecsId InId)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_FlecsId>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_FlecsId>().FlecsId = InId;
		
		this->GetQueryDefinition().AddQueryTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		return Get();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& With(const TSolidNotNull<const UScriptStruct*> InStruct)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = InStruct;
		
		this->AddTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		return Get();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& With(const FString& InString)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_String>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_String>().InputString = InString;
		
		this->GetQueryDefinition().AddQueryTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		return Get();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& With(const TSolidNotNull<const UEnum*> InEnum)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnum>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnum>().ScriptEnum = InEnum;
		
		this->GetQueryDefinition().AddQueryTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		return Get();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& With(const FSolidEnumSelector& InEnumSelector)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnumConstant>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnumConstant>().EnumValue = InEnumSelector;
		
		this->GetQueryDefinition().AddQueryTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		return Get();
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
			const FString TypeNameFString = FString(TypeName.data());
			this->With(TypeNameFString);
		}
		
		return Get();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Without(const FFlecsId InId)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_FlecsId>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_FlecsId>().FlecsId = InId;
		
		this->AddTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		this->Not();
		
		return Get();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Without(const TSolidNotNull<const UScriptStruct*> InStruct)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = InStruct;
		
		this->AddTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		this->Not();
		
		return Get();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Without(const FString& InString)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_String>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_String>().InputString = InString;
		
		this->AddTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		this->Not();
		
		return Get();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Without(const TSolidNotNull<const UEnum*> InEnum)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnum>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnum>().ScriptEnum = InEnum;
		
		this->AddTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		this->Not();
		
		return Get();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Without(const FSolidEnumSelector& InEnumSelector)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnumConstant>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnumConstant>().EnumValue = InEnumSelector;
		
		this->AddTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		this->Not();
		
		return Get();
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
			const FString TypeNameFString = FString(TypeName.data());
			this->Without(TypeNameFString);
		}
		
		this->Not();
		
		return Get();
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
		return Get();
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
		return Get();
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
		return Get();
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
		return Get();
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
		return Get();
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
		
		return Get();
	}
	
	template <Unreal::Flecs::Queries::CQueryDefinitionRecordInputType TFirst, Unreal::Flecs::Queries::CQueryDefinitionRecordInputType TSecond>
	FORCEINLINE_DEBUGGABLE FInheritedType& WithPair(const TFirst& InFirst, const TSecond& InSecond)
	{
		this->With(InFirst);
		this->Second(InSecond);
		return Get();
	}
	
	template <Unreal::Flecs::Queries::CQueryDefinitionRecordInputType TFirst, Unreal::Flecs::Queries::CQueryDefinitionRecordInputType TSecond>
	FORCEINLINE_DEBUGGABLE FInheritedType& WithoutPair(const TFirst& InFirst, const TSecond& InSecond)
	{
		this->Without(InFirst);
		this->Second(InSecond);
		return Get();
	}
	
	template <typename T, Unreal::Flecs::Queries::CQueryDefinitionRecordInputType TSecond>
	FORCEINLINE_DEBUGGABLE FInheritedType& WithPair(const TSecond& InSecond)
	{
		this->With<T>();
		this->Second(InSecond);
		return Get();
	}
	
	template <typename T, Unreal::Flecs::Queries::CQueryDefinitionRecordInputType TSecond>
	FORCEINLINE_DEBUGGABLE FInheritedType& WithoutPair(const TSecond& InSecond)
	{
		this->Without<T>();
		this->Second(InSecond);
		return Get();
	}
	
	template <typename T, Unreal::Flecs::Queries::CQueryDefinitionRecordInputType TFirst>
	FORCEINLINE_DEBUGGABLE FInheritedType& WithPairSecond(const TFirst& InFirst)
	{
		this->With(InFirst);
		this->Second<T>();
		return Get();
	}
	
	template <typename T, Unreal::Flecs::Queries::CQueryDefinitionRecordInputType TFirst>
	FORCEINLINE_DEBUGGABLE FInheritedType& WithoutPairSecond(const TFirst& InFirst)
	{
		this->Without(InFirst);
		this->Second<T>();
		return Get();
	}
	
	template <typename TFirst, typename TSecond>
	FORCEINLINE_DEBUGGABLE FInheritedType& WithPair()
	{
		this->With<TFirst>();
		this->Second<TSecond>();
		return Get();
	}
	
	template <typename TFirst, typename TSecond>
	FORCEINLINE_DEBUGGABLE FInheritedType& WithoutPair()
	{
		this->Without<TFirst>();
		this->Second<TSecond>();
		return Get();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& WithPair(const FSolidEnumSelector& InPair)
	{
		WithPair(InPair.Class, InPair.Value);
		return Get();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& WithoutPair(const FSolidEnumSelector& InPair)
	{
		WithoutPair(InPair.Class, InPair.Value);
		return Get();
	}
	
#pragma endregion TermHelperFunctions
	
	FORCEINLINE_DEBUGGABLE FInheritedType& Src(const FString& InSource)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		this->GetQueryDefinition().Terms[LastTermIndex].Source.template InitializeAs<FFlecsQueryGeneratorInputType_String>();
		this->GetQueryDefinition().Terms[LastTermIndex].Source.template GetMutable<FFlecsQueryGeneratorInputType_String>().InputString = InSource;
		return Get();
	}
	
	FORCEINLINE_DEBUGGABLE FInheritedType& ModifyLastTerm(const TFunctionRef<void(FFlecsQueryTermExpression&)>& InModifier)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		InModifier(this->GetQueryDefinition().Terms[LastTermIndex]);
		return Get();
	}
	
	template <Unreal::Flecs::Queries::TQueryExpressionConcept TExpression>
	FORCEINLINE_DEBUGGABLE FInheritedType& AddExpression(const TExpression& InExpression)
	{
		this->GetQueryDefinition().AddAdditionalExpression(InExpression);
		return Get();
	}
	
}; // struct TFlecsQueryBuilderBase
