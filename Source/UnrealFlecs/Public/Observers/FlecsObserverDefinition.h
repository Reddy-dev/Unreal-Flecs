// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "flecs.h"

#include "CoreMinimal.h"

#include "Entities/FlecsEntityHandle.h"

#include "FlecsObserverEventInput.h"
#include "FlecsObserverFlags.h"
#include "Queries/FlecsQueryDefinition.h"

#include "FlecsObserverDefinition.generated.h"

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsObserverDefinition
{
	GENERATED_BODY()
	
public:
	FORCEINLINE FFlecsObserverDefinition() = default;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FFlecsObserverEventInput> Events;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FFlecsQueryDefinition QueryDefinition;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bYieldExisting = false;
	
	UPROPERTY(EditAnywhere, meta = (Bitmask, BitmaskEnum = "/Script/UnrealFlecs.EFlecsObserverFlags"))
	uint32 Flags = static_cast<uint32>(EFlecsObserverFlags::None);
	
	UPROPERTY()
	bool bGlobalObserver = false;

	/** Callback to invoke on an event, invoked when the observer matches. */
	ecs_iter_action_t callback;

	/** Callback invoked on an event. When left to NULL the default runner
	 * is used which matches the event with the observer's query, and calls
	 * 'callback' when it matches.
	 * A reason to override the run function is to improve performance, if there
	 * are more efficient way to test whether an event matches the observer than
	 * the general purpose query matcher. */
	ecs_run_action_t run;

	/** User context to pass to callback */
	void *ctx;

	/** Callback to free ctx */
	ecs_ctx_free_t ctx_free;

	/** Context associated with callback (for language bindings). */
	void *callback_ctx;

	/** Callback to free callback ctx. */
	ecs_ctx_free_t callback_ctx_free;

	/** Context associated with run (for language bindings). */
	void *run_ctx;

	/** Callback to free run ctx. */
	ecs_ctx_free_t run_ctx_free;

	//ecs_observable_t *observable; /**< Observable for observer */
	
	void ApplyToObserver(const TSolidNotNull<const UFlecsWorld*> InFlecsWorld, flecs::observer_builder<>& InObserverBuilder) const;
	
}; // struct FFlecsObserverDefinition