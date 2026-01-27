// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsQueryFlags.h"
#include "Enums/FlecsQueryCache.h"
#include "Enums/FlecsQueryInOut.h"
#include "Expressions/FlecsExpressionInOut.h"
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
	
	FORCEINLINE FInheritedType& Get()
	{
		return static_cast<TInherited&>(*this);
	}
	
	FORCEINLINE const FInheritedType& Get() const
	{
		return static_cast<const TInherited&>(*this);
	}
	
	FORCEINLINE FFlecsQueryDefinition& GetQueryDefinition() const
	{
		return this->Get().GetQueryDefinition_Impl();
	}
	
public:
	mutable int32 LastTermIndex = -1;
	
	FORCEINLINE FInheritedType& AddTerm(const FFlecsQueryTermExpression& InTerm)
	{
		this->GetQueryDefinition().AddQueryTerm(InTerm);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		return Get();
	}
	
	FORCEINLINE FInheritedType& TermAt(const int32 InTermIndex)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		LastTermIndex = InTermIndex;
		return Get();
	}
	
#pragma region QueryDefinitionProperties
	
	FORCEINLINE FInheritedType& Cache(const EFlecsQueryCacheType InCacheType = EFlecsQueryCacheType::Default)
	{
		this->GetQueryDefinition().CacheType = InCacheType;
		return Get();
	}
	
	FORCEINLINE FInheritedType& DetectChanges(const bool bInDetectChanges = true)
	{
		this->GetQueryDefinition().bDetectChanges = bInDetectChanges;
		return Get();
	}
	
	FORCEINLINE FInheritedType& Flags(const uint8 InFlags)
	{
		this->GetQueryDefinition().Flags = InFlags;
		return Get();
	}
	
	FORCEINLINE FInheritedType& Flags(const EFlecsQueryFlags InFlags)
	{
		this->GetQueryDefinition().Flags = static_cast<uint8>(InFlags);
		return Get();
	}
	
#pragma endregion QueryDefinitionProperties
	
#pragma region TermOperatorExpressions
	
	FORCEINLINE FInheritedType& Oper(const EFlecsQueryOperator InOperator)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		
		this->GetQueryDefinition().Terms[LastTermIndex].Operator = InOperator;
		return Get();
	}
	
	FORCEINLINE FInheritedType& And()
	{
		return Oper(EFlecsQueryOperator::And);
	}
	
	FORCEINLINE FInheritedType& Or()
	{
		return Oper(EFlecsQueryOperator::Or);
	}
	
	FORCEINLINE FInheritedType& Not()
	{
		return Oper(EFlecsQueryOperator::Not);
	}
	
	FORCEINLINE FInheritedType& Optional()
	{
		return Oper(EFlecsQueryOperator::Optional);
	}
	
	FORCEINLINE FInheritedType& AndFrom()
	{
		return Oper(EFlecsQueryOperator::AndFrom);
	}
	
	FORCEINLINE FInheritedType& OrFrom()
	{
		return Oper(EFlecsQueryOperator::OrFrom);
	}
	
#pragma endregion TermOperatorExpressions
	
#pragma region ReadWriteInOutExpressions
	
	FORCEINLINE FInheritedType& InOutExpression(const EFlecsQueryInOut InInOut, const bool bStage = false)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		
		FFlecsExpressionInOut InOut;
		InOut.InOut = InInOut;
		InOut.bStage = bStage;
		
		this->GetQueryDefinition().Terms[LastTermIndex].Children.Add(TInstancedStruct<FFlecsQueryExpression>::Make(InOut));
		return Get();
	}
	
	FORCEINLINE FInheritedType& In()
	{
		return InOutExpression(EFlecsQueryInOut::Read, false);
	}
	
	FORCEINLINE FInheritedType& Out()
	{
		return InOutExpression(EFlecsQueryInOut::Write, false);
	}
	
	FORCEINLINE FInheritedType& InOut()
	{
		return InOutExpression(EFlecsQueryInOut::ReadWrite, false);
	}
	
	FORCEINLINE FInheritedType& Read()
	{
		return InOutExpression(EFlecsQueryInOut::Read, true);
	}
	
	FORCEINLINE FInheritedType& Write()
	{
		return InOutExpression(EFlecsQueryInOut::Write, true);
	}
	
	FORCEINLINE FInheritedType& ReadWrite()
	{
		return InOutExpression(EFlecsQueryInOut::ReadWrite, true);
	}
	
	FORCEINLINE FInheritedType& Filter()
	{
		return InOutExpression(EFlecsQueryInOut::Filter, false);
	}
	
	FORCEINLINE FInheritedType& InOutNone()
	{
		return InOutExpression(EFlecsQueryInOut::None, false);
	}
	
#pragma endregion ReadWriteInOutExpressions
	
#pragma region TermHelperFunctions
	
	FORCEINLINE FInheritedType& With(const FFlecsId InId)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_FlecsId>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_FlecsId>().FlecsId = InId;
		
		this->GetQueryDefinition().AddQueryTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		return Get();
	}
	
	FORCEINLINE FInheritedType& With(const TSolidNotNull<const UScriptStruct*> InStruct)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = InStruct;
		
		this->AddTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		return Get();
	}
	
	FORCEINLINE FInheritedType& With(const FString& InString)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_String>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_String>().InputString = InString;
		
		this->GetQueryDefinition().AddQueryTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		return Get();
	}
	
	FORCEINLINE FInheritedType& With(const TSolidNotNull<const UEnum*> InEnum)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnum>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnum>().ScriptEnum = InEnum;
		
		this->GetQueryDefinition().AddQueryTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		return Get();
	}
	
	FORCEINLINE FInheritedType& With(const FSolidEnumSelector& InEnumSelector)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnumConstant>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnumConstant>().EnumValue = InEnumSelector;
		
		this->GetQueryDefinition().AddQueryTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		return Get();
	}
	
	template <typename T>
	FORCEINLINE FInheritedType& With()
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
	
	FORCEINLINE FInheritedType& Without(const FFlecsId InId)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_FlecsId>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_FlecsId>().FlecsId = InId;
		this->Not();
		
		this->AddTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		return Get();
	}
	
	FORCEINLINE FInheritedType& Without(const TSolidNotNull<const UScriptStruct*> InStruct)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = InStruct;
		this->Not();
		
		this->AddTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		return Get();
	}
	
	FORCEINLINE FInheritedType& Without(const FString& InString)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_String>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_String>().InputString = InString;
		this->Not();
		
		this->AddTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		return Get();
	}
	
	FORCEINLINE FInheritedType& Without(const TSolidNotNull<const UEnum*> InEnum)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnum>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnum>().ScriptEnum = InEnum;
		this->Not();
		
		this->AddTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		return Get();
	}
	
	FORCEINLINE FInheritedType& Without(const FSolidEnumSelector& InEnumSelector)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnumConstant>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnumConstant>().EnumValue = InEnumSelector;
		this->Not();
		
		this->AddTerm(Expr);
		LastTermIndex = this->GetQueryDefinition().GetLastTermIndex();
		
		return Get();
	}
	
	template <typename T>
	FORCEINLINE FInheritedType& Without()
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
	
	FORCEINLINE FInheritedType& Second(const FFlecsId InId)
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
	
	FORCEINLINE FInheritedType& Second(const TSolidNotNull<const UScriptStruct*> InStruct)
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
	
	FORCEINLINE FInheritedType& Second(const FString& InString)
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
	
	FORCEINLINE FInheritedType& Second(const TSolidNotNull<const UEnum*> InEnum)
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
	
	FORCEINLINE FInheritedType& Second(const FSolidEnumSelector& InEnumSelector)
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
	FORCEINLINE FInheritedType& Second()
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
	FORCEINLINE FInheritedType& WithPair(const TFirst& InFirst, const TSecond& InSecond)
	{
		this->With(InFirst);
		this->Second(InSecond);
		return Get();
	}
	
	template <Unreal::Flecs::Queries::CQueryDefinitionRecordInputType TFirst, Unreal::Flecs::Queries::CQueryDefinitionRecordInputType TSecond>
	FORCEINLINE FInheritedType& WithoutPair(const TFirst& InFirst, const TSecond& InSecond)
	{
		this->Without(InFirst);
		this->Second(InSecond);
		return Get();
	}
	
	template <typename T, Unreal::Flecs::Queries::CQueryDefinitionRecordInputType TSecond>
	FORCEINLINE FInheritedType& WithPair(const TSecond& InSecond)
	{
		this->With<T>();
		this->Second(InSecond);
		return Get();
	}
	
	template <typename T, Unreal::Flecs::Queries::CQueryDefinitionRecordInputType TSecond>
	FORCEINLINE FInheritedType& WithoutPair(const TSecond& InSecond)
	{
		this->Without<T>();
		this->Second(InSecond);
		return Get();
	}
	
	template <typename T, Unreal::Flecs::Queries::CQueryDefinitionRecordInputType TFirst>
	FORCEINLINE FInheritedType& WithPairSecond(const TFirst& InFirst)
	{
		this->With(InFirst);
		this->Second<T>();
		return Get();
	}
	
	template <typename T, Unreal::Flecs::Queries::CQueryDefinitionRecordInputType TFirst>
	FORCEINLINE FInheritedType& WithoutPairSecond(const TFirst& InFirst)
	{
		this->Without(InFirst);
		this->Second<T>();
		return Get();
	}
	
	template <typename TFirst, typename TSecond>
	FORCEINLINE FInheritedType& WithPair()
	{
		this->With<TFirst>();
		this->Second<TSecond>();
		return Get();
	}
	
	template <typename TFirst, typename TSecond>
	FORCEINLINE FInheritedType& WithoutPair()
	{
		this->Without<TFirst>();
		this->Second<TSecond>();
		return Get();
	}
	
	FORCEINLINE FInheritedType& WithPair(const FSolidEnumSelector& InPair)
	{
		WithPair(InPair.Class, InPair.Value);
		return Get();
	}
	
	FORCEINLINE FInheritedType& WithoutPair(const FSolidEnumSelector& InPair)
	{
		WithoutPair(InPair.Class, InPair.Value);
		return Get();
	}
	
#pragma endregion TermHelperFunctions
	
	FORCEINLINE FInheritedType& ModifyLastTerm(const TFunctionRef<void(FFlecsQueryTermExpression&)>& InModifier)
	{
		solid_checkf(this->GetQueryDefinition().IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		InModifier(this->GetQueryDefinition().Terms[LastTermIndex]);
		return Get();
	}
	
	template <Unreal::Flecs::Queries::TQueryExpressionConcept TExpression>
	FORCEINLINE FInheritedType& AddExpression(const TExpression& InExpression)
	{
		this->GetQueryDefinition().AddAdditionalExpression(InExpression);
		return Get();
	}
	
}; // struct TFlecsQueryBuilderBase
