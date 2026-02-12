// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Queries/Expressions/FlecsQueryOrderByExpression.h"

#include "Queries/Callbacks/FlecsOrderByCallbackDefinition.h"
#include "Queries/Generator/FlecsQueryGeneratorInputType.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsQueryOrderByExpression)

FFlecsQueryOrderByExpression::FFlecsQueryOrderByExpression() : Super(false /* bInAllowsChildExpressions */)
{
}

void FFlecsQueryOrderByExpression::Apply(const TSolidNotNull<const UFlecsWorld*> InWorld,
	flecs::query_builder<>& InQueryBuilder) const
{
	FFlecsId OrderByComponentId;
	if LIKELY_IF(OrderByInput.Get().ReturnType == EFlecsQueryGeneratorReturnType::FlecsId)
	{
		OrderByComponentId = OrderByInput.Get<FFlecsQueryGeneratorInputType>().GetFlecsIdOutput(InWorld);
	}
	else
	{
		solid_checkf(false, TEXT("Unsupported OrderBy input type return type"));
	}
	
	if UNLIKELY_IF(!ensureAlwaysMsgf(OrderByCallback.IsValid(), TEXT("Invalid OrderBy callback provided!")))
	{
		return;
	}
	
	InQueryBuilder.order_by(OrderByComponentId, 
		Unreal::Flecs::Queries::MakeOrderByFunction(OrderByCallback.Get<FFlecsOrderByCallbackDefinition>().GetOrderByFunction()));
}

FFlecsQueryOrderByCPPExpressionWrapper::FFlecsQueryOrderByCPPExpressionWrapper() : Super(false /* bInAllowsChildExpressions */)
{
}

void FFlecsQueryOrderByCPPExpressionWrapper::Apply(const TSolidNotNull<const UFlecsWorld*> InWorld,
	flecs::query_builder<>& InQueryBuilder) const
{
	FFlecsId OrderByComponentId;
	if LIKELY_IF(OrderByInput.Get().ReturnType == EFlecsQueryGeneratorReturnType::FlecsId)
	{
		OrderByComponentId = OrderByInput.Get<FFlecsQueryGeneratorInputType>().GetFlecsIdOutput(InWorld);
	}
	else
	{
		solid_checkf(false, TEXT("Unsupported OrderBy input type return type"));
	}
	
	if UNLIKELY_IF(!ensureAlwaysMsgf(OrderByFunction, TEXT("Invalid OrderBy function provided!")))
	{
		return;
	}
	
	InQueryBuilder.order_by(OrderByComponentId, Unreal::Flecs::Queries::MakeOrderByFunction(OrderByFunction));
}
