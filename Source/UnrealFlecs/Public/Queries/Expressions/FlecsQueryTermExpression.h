// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Queries/Enums/FlecsQueryOperators.h"
#include "FlecsQueryExpression.h"
#include "Queries/FlecsQueryTerm.h"
#include "Queries/Enums/FlecsQueryInOut.h"

#include "FlecsQueryTermExpression.generated.h"

USTRUCT(BlueprintType, meta = (DisplayName = "Term Query"))
struct UNREALFLECS_API FFlecsQueryTermExpression : public FFlecsQueryExpression
{
	GENERATED_BODY()

public:
	FFlecsQueryTermExpression();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
	FFlecsQueryTerm Term;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Query")
	EFlecsQueryOperator Operator = EFlecsQueryOperator::Default;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
	TInstancedStruct<FFlecsQueryGeneratorInputType> Source;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
	EFlecsQueryInOut InOut = EFlecsQueryInOut::Default;

	/** Set read/write access for stage. Use this when a system reads or writes
	 * components other than the ones provided by the query. This information 
	 * can be used by schedulers to insert sync/merge points between systems
	 * where deferred operations are flushed.
	 * 
	 * Setting this is optional. If not set, the value of the accessed component
	 * may be out of sync for at most one frame.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Query")
	bool bStage = false;

	virtual void Apply(const TSolidNotNull<const UFlecsWorld*> InWorld, flecs::query_builder<>& InQueryBuilder) const override;
	
}; // struct FFlecsQueryTermExpression
