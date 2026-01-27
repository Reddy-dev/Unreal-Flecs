// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Entities/FlecsEntityRecord.h"
#include "FlecsQueryDefinition.h"
#include "Enums/FlecsQueryInOut.h"
#include "Expressions/FlecsExpressionInOut.h"
#include "Generator/FlecsQueryGeneratorInputType.h"

#include "FlecsQueryDefinitionRecordFragment.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsQueryDefinitionRecordFragment : public FFlecsEntityRecordFragment
{
	GENERATED_BODY()
	
public:
	FFlecsQueryDefinitionRecordFragment() = default;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flecs | Query Definition")
	FFlecsQueryDefinition QueryDefinition;
	
	virtual void PreApplyRecordToEntity(const TSolidNotNull<const UFlecsWorld*> InFlecsWorld, const FFlecsEntityHandle& InEntityHandle) const override;
	virtual void PostApplyRecordToEntity(const TSolidNotNull<const UFlecsWorld*> InFlecsWorld, const FFlecsEntityHandle& InEntityHandle) const override;
	
	struct FBuilder;
	
}; // struct FFlecsQueryDefinitionRecordFragment

namespace Unreal::Flecs::Queries
{
	template <typename T>
	concept CQueryDefinitionRecordInputType = std::is_convertible<T, FFlecsId>::value
		|| std::is_convertible<T, const UScriptStruct*>::value
		|| std::is_convertible<T, FString>::value
		|| std::is_convertible<T, const UEnum*>::value
		|| std::is_convertible<T, FSolidEnumSelector>::value;
	
} // namespace Unreal::Flecs::Queries

struct FFlecsQueryDefinitionRecordFragment::FBuilder : public FFlecsEntityRecord::FFragmentBuilderType<FFlecsQueryDefinitionRecordFragment>
{
	using Super = FFlecsEntityRecord::FFragmentBuilderType<FFlecsQueryDefinitionRecordFragment>;
	using Super::Super;
	
	mutable int32 LastTermIndex = -1;
	
public:
	FORCEINLINE FBuilder& AddTerm(const FFlecsQueryTermExpression& InTerm)
	{
		this->Get().QueryDefinition.AddQueryTerm(InTerm);
		LastTermIndex = this->Get().QueryDefinition.GetLastTermIndex();
		return *this;
	}
	
	FORCEINLINE FBuilder& TermAt(const int32 InTermIndex)
	{
		solid_checkf(this->Get().QueryDefinition.IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		LastTermIndex = InTermIndex;
		return *this;
	}
	
#pragma region QueryDefinitionProperties
	
	FORCEINLINE FBuilder& Cache(const EFlecsQueryCacheType InCacheType = EFlecsQueryCacheType::Default)
	{
		this->Get().QueryDefinition.CacheType = InCacheType;
		return *this;
	}
	
	FORCEINLINE FBuilder& DetectChanges(const bool bInDetectChanges = true)
	{
		this->Get().QueryDefinition.bDetectChanges = bInDetectChanges;
		return *this;
	}
	
	FORCEINLINE FBuilder& Flags(const uint8 InFlags)
	{
		this->Get().QueryDefinition.Flags = InFlags;
		return *this;
	}
	
	FORCEINLINE FBuilder& Flags(const EFlecsQueryFlags InFlags)
	{
		this->Get().QueryDefinition.Flags = static_cast<uint8>(InFlags);
		return *this;
	}
	
#pragma endregion QueryDefinitionProperties
	
#pragma region TermOperatorExpressions
	
	FORCEINLINE FBuilder& Oper(const EFlecsQueryOperator InOperator)
	{
		solid_checkf(this->Get().QueryDefinition.IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		
		this->Get().QueryDefinition.Terms[LastTermIndex].Operator = InOperator;
		return *this;
	}
	
	FORCEINLINE FBuilder& And()
	{
		return Oper(EFlecsQueryOperator::And);
	}
	
	FORCEINLINE FBuilder& Or()
	{
		return Oper(EFlecsQueryOperator::Or);
	}
	
	FORCEINLINE FBuilder& Not()
	{
		return Oper(EFlecsQueryOperator::Not);
	}
	
	FORCEINLINE FBuilder& Optional()
	{
		return Oper(EFlecsQueryOperator::Optional);
	}
	
	FORCEINLINE FBuilder& AndFrom()
	{
		return Oper(EFlecsQueryOperator::AndFrom);
	}
	
	FORCEINLINE FBuilder& OrFrom()
	{
		return Oper(EFlecsQueryOperator::OrFrom);
	}
	
#pragma endregion TermOperatorExpressions
	
#pragma region ReadWriteInOutExpressions
	
	FORCEINLINE FBuilder& InOutExpression(const EFlecsQueryInOut InInOut, const bool bStage = false)
	{
		solid_checkf(this->Get().QueryDefinition.IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		
		FFlecsExpressionInOut InOut;
		InOut.InOut = InInOut;
		InOut.bStage = bStage;
		
		this->Get().QueryDefinition.Terms[LastTermIndex].Children.Add(TInstancedStruct<FFlecsQueryExpression>::Make(InOut));
		return *this;
	}
	
	FORCEINLINE FBuilder& In()
	{
		return InOutExpression(EFlecsQueryInOut::Read, false);
	}
	
	FORCEINLINE FBuilder& Out()
	{
		return InOutExpression(EFlecsQueryInOut::Write, false);
	}
	
	FORCEINLINE FBuilder& InOut()
	{
		return InOutExpression(EFlecsQueryInOut::ReadWrite, false);
	}
	
	FORCEINLINE FBuilder& Read()
	{
		return InOutExpression(EFlecsQueryInOut::Read, true);
	}
	
	FORCEINLINE FBuilder& Write()
	{
		return InOutExpression(EFlecsQueryInOut::Write, true);
	}
	
	FORCEINLINE FBuilder& ReadWrite()
	{
		return InOutExpression(EFlecsQueryInOut::ReadWrite, true);
	}
	
	FORCEINLINE FBuilder& Filter()
	{
		return InOutExpression(EFlecsQueryInOut::Filter, false);
	}
	
#pragma endregion ReadWriteInOutExpressions
	
	FORCEINLINE FBuilder& With(const FFlecsId InId)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_FlecsId>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_FlecsId>().FlecsId = InId;
		
		this->Get().QueryDefinition.AddQueryTerm(Expr);
		LastTermIndex = this->Get().QueryDefinition.GetLastTermIndex();
		
		return *this;
	}
	
	FORCEINLINE FBuilder& With(const TSolidNotNull<const UScriptStruct*> InStruct)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = InStruct;
		
		this->AddTerm(Expr);
		LastTermIndex = this->Get().QueryDefinition.GetLastTermIndex();
		
		return *this;
	}
	
	FORCEINLINE FBuilder& With(const FString& InString)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_String>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_String>().InputString = InString;
		
		this->Get().QueryDefinition.AddQueryTerm(Expr);
		LastTermIndex = this->Get().QueryDefinition.GetLastTermIndex();
		
		return *this;
	}
	
	FORCEINLINE FBuilder& With(const TSolidNotNull<const UEnum*> InEnum)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnum>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnum>().ScriptEnum = InEnum;
		
		this->Get().QueryDefinition.AddQueryTerm(Expr);
		LastTermIndex = this->Get().QueryDefinition.GetLastTermIndex();
		
		return *this;
	}
	
	FORCEINLINE FBuilder& With(const FSolidEnumSelector& InEnumSelector)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnumConstant>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnumConstant>().EnumValue = InEnumSelector;
		
		this->Get().QueryDefinition.AddQueryTerm(Expr);
		LastTermIndex = this->Get().QueryDefinition.GetLastTermIndex();
		
		return *this;
	}
	
	template <typename T>
	FORCEINLINE FBuilder& With()
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
		
		return *this;
	}
	
	FORCEINLINE FBuilder& Without(const FFlecsId InId)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_FlecsId>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_FlecsId>().FlecsId = InId;
		this->Not();
		
		this->AddTerm(Expr);
		LastTermIndex = this->Get().QueryDefinition.GetLastTermIndex();
		
		return *this;
	}
	
	FORCEINLINE FBuilder& Without(const TSolidNotNull<const UScriptStruct*> InStruct)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = InStruct;
		this->Not();
		
		this->AddTerm(Expr);
		LastTermIndex = this->Get().QueryDefinition.GetLastTermIndex();
		
		return *this;
	}
	
	FORCEINLINE FBuilder& Without(const FString& InString)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_String>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_String>().InputString = InString;
		this->Not();
		
		this->AddTerm(Expr);
		LastTermIndex = this->Get().QueryDefinition.GetLastTermIndex();
		
		return *this;
	}
	
	FORCEINLINE FBuilder& Without(const TSolidNotNull<const UEnum*> InEnum)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnum>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnum>().ScriptEnum = InEnum;
		this->Not();
		
		this->AddTerm(Expr);
		LastTermIndex = this->Get().QueryDefinition.GetLastTermIndex();
		
		return *this;
	}
	
	FORCEINLINE FBuilder& Without(const FSolidEnumSelector& InEnumSelector)
	{
		FFlecsQueryTermExpression Expr;
		Expr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnumConstant>();
		Expr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnumConstant>().EnumValue = InEnumSelector;
		this->Not();
		
		this->AddTerm(Expr);
		LastTermIndex = this->Get().QueryDefinition.GetLastTermIndex();
		
		return *this;
	}
	
	template <typename T>
	FORCEINLINE FBuilder& Without()
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
		
		return *this;
	}
	
	FORCEINLINE FBuilder& Second(const FFlecsId InId)
	{
		solid_checkf(this->Get().QueryDefinition.IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		
		FFlecsQueryTermExpression SecondExpr;
		SecondExpr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_FlecsId>();
		SecondExpr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_FlecsId>().FlecsId = InId;
		
		FFlecsQueryTermExpression& TermExpr = this->Get().QueryDefinition.Terms[LastTermIndex];
		
		const FFlecsQueryTerm FirstTerm = TermExpr.Term;
		
		TermExpr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_Pair>();
		TermExpr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_Pair>().First = FirstTerm.InputType;
		TermExpr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_Pair>().Second = SecondExpr.Term.InputType;
		return *this;
	}
	
	FORCEINLINE FBuilder& Second(const TSolidNotNull<const UScriptStruct*> InStruct)
	{
		solid_checkf(this->Get().QueryDefinition.IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		
		FFlecsQueryTermExpression SecondExpr;
		SecondExpr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_ScriptStruct>();
		SecondExpr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_ScriptStruct>().ScriptStruct = InStruct;
		
		FFlecsQueryTermExpression& TermExpr = this->Get().QueryDefinition.Terms[LastTermIndex];
		
		const FFlecsQueryTerm FirstTerm = TermExpr.Term;
		
		TermExpr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_Pair>();
		TermExpr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_Pair>().First = FirstTerm.InputType;
		TermExpr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_Pair>().Second = SecondExpr.Term.InputType;
		return *this;
	}
	
	FORCEINLINE FBuilder& Second(const FString& InString)
	{
		solid_checkf(this->Get().QueryDefinition.IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		
		FFlecsQueryTermExpression SecondExpr;
		SecondExpr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_String>();
		SecondExpr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_String>().InputString = InString;
		
		FFlecsQueryTermExpression& TermExpr = this->Get().QueryDefinition.Terms[LastTermIndex];
		
		const FFlecsQueryTerm FirstTerm = TermExpr.Term;
		
		TermExpr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_Pair>();
		TermExpr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_Pair>().First = FirstTerm.InputType;
		TermExpr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_Pair>().Second = SecondExpr.Term.InputType;
		return *this;
	}
	
	FORCEINLINE FBuilder& Second(const TSolidNotNull<const UEnum*> InEnum)
	{
		solid_checkf(this->Get().QueryDefinition.IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		
		FFlecsQueryTermExpression SecondExpr;
		SecondExpr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnum>();
		SecondExpr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnum>().ScriptEnum = InEnum;
		
		FFlecsQueryTermExpression& TermExpr = this->Get().QueryDefinition.Terms[LastTermIndex];
		
		const FFlecsQueryTerm FirstTerm = TermExpr.Term;
		
		TermExpr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_Pair>();
		TermExpr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_Pair>().First = FirstTerm.InputType;
		TermExpr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_Pair>().Second = SecondExpr.Term.InputType;
		return *this;
	}
	
	FORCEINLINE FBuilder& Second(const FSolidEnumSelector& InEnumSelector)
	{
		solid_checkf(this->Get().QueryDefinition.IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		
		FFlecsQueryTermExpression SecondExpr;
		SecondExpr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_ScriptEnumConstant>();
		SecondExpr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_ScriptEnumConstant>().EnumValue = InEnumSelector;
		
		FFlecsQueryTermExpression& TermExpr = this->Get().QueryDefinition.Terms[LastTermIndex];
		const FFlecsQueryTerm FirstTerm = TermExpr.Term;
		TermExpr.Term.InputType.InitializeAs<FFlecsQueryGeneratorInputType_Pair>();
		TermExpr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_Pair>().First = FirstTerm.InputType;
		TermExpr.Term.InputType.GetMutable<FFlecsQueryGeneratorInputType_Pair>().Second = SecondExpr.Term.InputType;
		return *this;
	}
	
	template <typename T>
	FORCEINLINE FBuilder& Second()
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
		
		return *this;
	}
	
	template <Unreal::Flecs::Queries::CQueryDefinitionRecordInputType TFirst, Unreal::Flecs::Queries::CQueryDefinitionRecordInputType TSecond>
	FORCEINLINE FBuilder& WithPair(const TFirst& InFirst, const TSecond& InSecond)
	{
		this->With(InFirst);
		this->Second(InSecond);
		return *this;
	}
	
	template <Unreal::Flecs::Queries::CQueryDefinitionRecordInputType TFirst, Unreal::Flecs::Queries::CQueryDefinitionRecordInputType TSecond>
	FORCEINLINE FBuilder& WithoutPair(const TFirst& InFirst, const TSecond& InSecond)
	{
		this->Without(InFirst);
		this->Second(InSecond);
		return *this;
	}
	
	template <typename T, Unreal::Flecs::Queries::CQueryDefinitionRecordInputType TSecond>
	FORCEINLINE FBuilder& WithPair(const TSecond& InSecond)
	{
		this->With<T>();
		this->Second(InSecond);
		return *this;
	}
	
	template <typename T, Unreal::Flecs::Queries::CQueryDefinitionRecordInputType TSecond>
	FORCEINLINE FBuilder& WithoutPair(const TSecond& InSecond)
	{
		this->Without<T>();
		this->Second(InSecond);
		return *this;
	}
	
	template <typename T, Unreal::Flecs::Queries::CQueryDefinitionRecordInputType TFirst>
	FORCEINLINE FBuilder& WithPairSecond(const TFirst& InFirst)
	{
		this->With<T>();
		this->Second(InFirst);
		return *this;
	}
	
	template <typename T, Unreal::Flecs::Queries::CQueryDefinitionRecordInputType TFirst>
	FORCEINLINE FBuilder& WithoutPairSecond(const TFirst& InFirst)
	{
		this->Without<T>();
		this->Second(InFirst);
		return *this;
	}
	
	template <typename TFirst, typename TSecond>
	FORCEINLINE FBuilder& WithPair()
	{
		this->With<TFirst>();
		this->Second<TSecond>();
		return *this;
	}
	
	template <typename TFirst, typename TSecond>
	FORCEINLINE FBuilder& WithoutPair()
	{
		this->Without<TFirst>();
		this->Second<TSecond>();
		return *this;
	}
	
	FORCEINLINE FBuilder& ModifyLastTerm(const TFunctionRef<void(FFlecsQueryTermExpression&)>& InModifier)
	{
		solid_checkf(this->Get().QueryDefinition.IsValidTermIndex(LastTermIndex), TEXT("Invalid term index provided"));
		InModifier(this->Get().QueryDefinition.Terms[LastTermIndex]);
		return *this;
	}
	
	template <Unreal::Flecs::Queries::TQueryExpressionConcept TExpression>
	FORCEINLINE FBuilder& AddExpression(const TExpression& InExpression)
	{
		this->Get().QueryDefinition.AddAdditionalExpression(InExpression);
		return *this;
	}
	
}; // struct FFlecsQueryDefinitionRecordFragment::FBuilder
