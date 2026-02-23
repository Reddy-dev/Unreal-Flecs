// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Queries/Enums/FlecsQueryOperators.h"
#include "FlecsQueryExpression.h"
#include "Queries/FlecsQueryTerm.h"
#include "Queries/Enums/FlecsQueryInOut.h"
#include "Queries/Generator/FlecsQueryGeneratorInput.h"

#include "FlecsQueryTermExpression.generated.h"

USTRUCT(BlueprintType, meta = (DisplayName = "Term Query"))
struct UNREALFLECS_API FFlecsQueryTermExpression : public FFlecsQueryExpression
{
	GENERATED_BODY()

public:
	FFlecsQueryTermExpression();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FFlecsQueryTerm Term;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EFlecsQueryOperator Operator = EFlecsQueryOperator::Default;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowsPairInput = false))
	FFlecsQueryGeneratorInput Source;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFlecsQueryInOut InOut = EFlecsQueryInOut::Default;

	/** Set read/write access for stage. Use this when a system reads or writes
	 * components other than the ones provided by the query. This information 
	 * can be used by schedulers to insert sync/merge points between systems
	 * where deferred operations are flushed.
	 * 
	 * Setting this is optional. If not set, the value of the accessed component
	 * may be out of sync for at most one frame.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bStage = false;

	virtual void Apply(const TSolidNotNull<const UFlecsWorld*> InWorld, FFlecsQueryBuilderView& InQueryBuilder) const override;
	
}; // struct FFlecsQueryTermExpression
