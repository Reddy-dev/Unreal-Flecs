// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "flecs.h"

#include "CoreMinimal.h"

#include "Queries/FlecsQueryDefinition.h"
#include "FlecsSystemPhaseInput.h"
#include "FlecsSystemPipelineInput.h"
#include "FlecsSystemTickSourceInput.h"

#include "FlecsSystemDefinition.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsSystemDefinition
{
	GENERATED_BODY()
	
public:
	FORCEINLINE FFlecsSystemDefinition() = default;
	
	UPROPERTY(EditAnywhere)
	FFlecsQueryDefinition QueryDefinition;
	
	UPROPERTY(EditAnywhere)
	FFlecsSystemPhaseInput PhaseInput;
		
	UPROPERTY(EditAnywhere)
	double Interval = 0.0;
	
	UPROPERTY(EditAnywhere)
	uint32 Rate;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FFlecsSystemTickSourceInput TickSourceInput;
	
	UPROPERTY(EditAnywhere)
	bool bMultiThreaded = false;
	
	UPROPERTY(EditAnywhere)
	bool bImmediate = false;
	
	UPROPERTY(EditAnywhere)
	FFlecsSystemPipelineInput PipelineInput;
	
	ecs_iter_action_t callback;

	/** Callback that is invoked when a system is ran.
	 * When left to NULL, the default system runner is used, which calls the
	 * "callback" action for each result returned from the system's query.
	 *
	 * It should not be assumed that the input iterator can always be iterated
	 * with ecs_query_next(). When a system is multithreaded and/or paged, the
	 * iterator can be either a worker or paged iterator. The correct function 
	 * to use for iteration is ecs_iter_next().
	 *
	 * An implementation can test whether the iterator is a query iterator by
	 * testing whether the it->next value is equal to ecs_query_next(). */
	ecs_run_action_t run;

	/** Context to be passed to callback (as ecs_iter_t::param) */
	void *ctx;

	/** Callback to free ctx. */
	ecs_ctx_free_t ctx_free;

	/** Context associated with callback (for language bindings). */
	void *callback_ctx;

	/** Callback to free callback ctx. */
	ecs_ctx_free_t callback_ctx_free;

	/** Context associated with run (for language bindings). */
	void *run_ctx;

	/** Callback to free run ctx. */
	ecs_ctx_free_t run_ctx_free;
	
	void ApplyToSystem(const TSolidNotNull<const UFlecsWorld*> InFlecsWorld, flecs::system_builder<>& InSystemBuilder) const;
	
}; // struct FFlecsSystemDefinition
