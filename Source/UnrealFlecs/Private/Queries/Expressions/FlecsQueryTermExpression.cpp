// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Queries/Expressions/FlecsQueryTermExpression.h"

#include "Worlds/FlecsWorld.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsQueryTermExpression)

FFlecsQueryTermExpression::FFlecsQueryTermExpression() : Super(true /* bInAllowsChildExpressions */)
{
}

void FFlecsQueryTermExpression::Apply(TSolidNotNull<const UFlecsWorld*> InWorld, flecs::query_builder<>& InQueryBuilder) const
{
	Term.ApplyToQueryBuilder(InWorld, InQueryBuilder);
	
	InQueryBuilder.oper(ToFlecsOperator(Operator));
	
	if (Source.IsValid())
	{
		const FFlecsQueryGeneratorInputType& SourceInputType = Source.Get<FFlecsQueryGeneratorInputType>();
		
		if (SourceInputType.ReturnType == EFlecsQueryGeneratorReturnType::FlecsId)
		{
			InQueryBuilder.src(SourceInputType.GetFlecsIdOutput(InWorld));
		}
		else if (SourceInputType.ReturnType == EFlecsQueryGeneratorReturnType::String)
		{
			const FString StringOutput = SourceInputType.GetStringOutput();
	
			const char* CStr = TCHAR_TO_UTF8(*StringOutput);
			solid_cassume(CStr != nullptr);
	
			const uint32 Length = static_cast<uint32>(FCStringAnsi::Strlen(CStr));
		
			const char* OwnedCStr = (const char*)FMemory::Malloc(Length + 1);
			solid_cassume(OwnedCStr != nullptr);
		
			FMemory::Memcpy((void*)OwnedCStr, (const void*)CStr, Length + 1);
			
			InQueryBuilder.src(OwnedCStr);
		}
		else
		{
			solid_checkf(false, TEXT("Unsupported source input type return type"));
		}
	}
	
	if (bStage)
	{
		InQueryBuilder.inout_stage(ToFlecsInOut(InOut));
	}
	else 
	{
		InQueryBuilder.inout(ToFlecsInOut(InOut));
	}
	
	Super::Apply(InWorld, InQueryBuilder);
}