// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "FlecsSystemBuilder.h"

#include "UObject/Object.h"

#include "FlecsSystemHandleInterface.h"
#include "General/FlecsObjectRegistrationInterface.h"
#include "Queries/FlecsIteratorObjectInterface.h"
#include "FlecsSystemDefinition.h"

#include "FlecsSystemObject.generated.h"

USTRUCT(BlueprintType)
struct FFlecsSystemDefinitionOverrides
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	TOptional<FFlecsSystemPhaseInput> PhaseInputOverride;
	
	UPROPERTY(EditAnywhere)
	TOptional<double> IntervalOverride;
	
	UPROPERTY(EditAnywhere)
	TOptional<uint32> RateOverride;
	
	UPROPERTY(EditAnywhere)
	TOptional<FFlecsSystemTickSourceInput> TickSourceInputOverride;
	
	/** Overrides whether the scheduler may split this system across worker stages. */
	UPROPERTY(EditAnywhere)
	TOptional<bool> MultiThreadedOverride;
	
	/** Overrides whether scheduled execution runs outside Flecs readonly mode. */
	UPROPERTY(EditAnywhere)
	TOptional<bool> ImmediateOverride;
	
	UPROPERTY(EditAnywhere)
	TOptional<FFlecsSystemPipelineInput> PipelineInputOverride;
	
}; // struct FFlecsSystemDefinitionOverrides

/**
 * UObject-backed Flecs system registered with each applicable Flecs world.
 *
 * The system is created during FlecsWorldBeginPlay. InitializeSystem creates a
 * builder from SystemDefinition, calls BuildSystem, applies configured
 * overrides, creates the native Flecs system, and finally calls OnBuildSystem.
 * Scheduled execution then enters IFlecsIteratorObjectInterface::RunIterator.
 *
 * A multithreaded system can be invoked in parallel on Flecs worker stages.
 * Its callback receives the matching stage through InWorld; it must not assume
 * game-thread execution or access non-thread-safe UObject/gameplay state.
 */
UCLASS(Abstract, BlueprintType, NotBlueprintable, Config = Flecs, DefaultConfig)
class UNREALFLECS_API UFlecsSystemObject : public UObject, public IFlecsSystemHandleInterface
	, public IFlecsIteratorObjectInterface, public IFlecsObjectRegistrationInterface
{
	GENERATED_BODY()

public:
	UFlecsSystemObject();
	UFlecsSystemObject(const FObjectInitializer& ObjectInitializer);
	
	NO_DISCARD FORCEINLINE virtual FFlecsSystemHandle GetSystemHandle() const override final
	{
		return SystemHandle;
	}
	
	/**
	 * Configures the query and scheduling properties before the native system is created.
	 *
	 * Calls to With, Without, and other query-term methods append terms in order.
	 * That zero-based order is also used by flecs::iter::field_at and is_set in
	 * iterator callbacks. The builder is valid only for this call and must not be
	 * retained. Config-backed SystemDefinitionOverrides are applied after this
	 * function returns and therefore take precedence over matching builder values.
	 *
	 * @param InWorld World used to resolve component types and other inputs.
	 * @param InBuilder Builder to configure for this system instance.
	 */
	virtual void BuildSystem(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld, TFlecsSystemBuilder<>& InBuilder) const;
	
	UFUNCTION(BlueprintCallable, Category = "Flecs|Observer")
	UFlecsWorld* GetFlecsWorld() const;
	
	virtual void RegisterObject(const TSolidNotNull<UFlecsWorldInterfaceObject*> InFlecsWorld) override;
	virtual void UnregisterObject(const TSolidNotNull<UFlecsWorldInterfaceObject*> InFlecsWorld) override;
	virtual void FlecsWorldBeginPlay(const TSolidNotNull<UFlecsWorldInterfaceObject*> InFlecsWorld) override;
	
	virtual NO_DISCARD uint8 GetObjectRegistrationNetworkFlags() const override
	{
		return NetworkRegistrationFlags;
	}
	
	/**
	 * Sets the unowned native Flecs system context pointer.
	 *
	 * The caller is responsible for keeping InContext alive until it is replaced
	 * or the system is destroyed. Unreal-Flecs does not free this pointer.
	 */
	void SetContext(void* InContext) const;
	
	/**
	 * Runs the native Flecs system synchronously with the supplied delta and parameter.
	 *
	 * This is an explicit run and does not wait for the system's configured phase.
	 * It also does not use bStartsDisabled or the enabled state as a guard. Flecs
	 * still evaluates the system query and tick source and opens a defer scope for
	 * the run. InParams is borrowed and is exposed as flecs::iter::param only for
	 * the duration of this call; it is not retained. When InParams is null, Flecs
	 * exposes the system context set by SetContext as the iterator parameter.
	 *
	 * @param InDeltaTime Delta time exposed by the system iterator.
	 * @param InParams Optional caller-owned parameter exposed as iterator param.
	 */
	void RunSystem(const double InDeltaTime = 0.0, void* InParams = nullptr) const;
	
	NO_DISCARD FORCEINLINE FFlecsSystemDefinition& GetSystemDefinition()
	{
		return SystemDefinition;
	}
	
	NO_DISCARD FORCEINLINE const FFlecsSystemDefinition& GetSystemDefinition() const
	{
		return SystemDefinition;
	}
	
	/**
	 * @brief Enable the system for scheduled execution in the world. (Enables the System entity itself)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs|System")
	void EnableSystem() const;
	
	/**
	 * @brief Disable the system for scheduled execution in the world. (Disables the System entity itself)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Flecs|System")
	void DisableSystem() const;
	
	
	/**
	 * @brief 
	 * @return True if the system is enabled for scheduled execution in the world. (Checks if the System entity itself is enabled)
	 */
	UFUNCTION(BlueprintCallable, Category = "Flecs|System")
	bool IsSystemEnabled() const;
	
#if WITH_EDITORONLY_DATA
	
	virtual NO_DISCARD bool ShouldShowInSettings() const override
	{
		return true;
	}
	
#endif // WITH_EDITORONLY_DATA
	
protected:
	UPROPERTY(Transient)
	FFlecsSystemHandle SystemHandle;
	
	UPROPERTY(EditDefaultsOnly, Config, Category = "Flecs", meta = (AllowPrivateAccess = "true", 
		Bitmask, BitmaskEnum = "/Script/UnrealFlecs.EFlecsObjectRegistrationNetworkFlags"))
	uint8 NetworkRegistrationFlags = static_cast<uint8>(EFlecsObjectRegistrationNetworkFlags::All);
	
	UPROPERTY(EditDefaultsOnly, Config, Category = "Flecs", meta = (AllowPrivateAccess = "true"))
	FFlecsSystemDefinitionOverrides SystemDefinitionOverrides;

	UPROPERTY(EditDefaultsOnly, Config, Category = "Flecs", meta = (AllowPrivateAccess = "true"))
	FFlecsSystemDefinition SystemDefinition;
	
	/**
	 * Disables the native system immediately after creation.
	 *
	 * A disabled system is skipped by scheduled pipeline execution until enabled.
	 * It can still be executed explicitly with RunSystem.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Config, Category = "Flecs", meta = (AllowPrivateAccess = "true"))
	uint8 bStartsDisabled : 1 = false;
	
	void ApplySystemDefinitionOverrides(FFlecsSystemDefinition& InOutDefinition) const;
	
	/**
	 * @brief 
	 * @param InSystemHandle 
	 */
	virtual void OnBuildSystem(const FFlecsSystemHandle& InSystemHandle);
	
private:
	void InitializeSystem(const TSolidNotNull<const UFlecsWorldInterfaceObject*> InWorld);
	
}; // class UFlecsSystemObject
