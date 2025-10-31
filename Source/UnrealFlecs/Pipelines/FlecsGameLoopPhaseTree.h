// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "flecs.h"

#include "CoreMinimal.h"

#include "StructUtils/InstancedStruct.h"
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"

#include "Types/SolidNotNull.h"

#include "Entities/FlecsEntityHandle.h"
#include "FlecsDefaultPhases.h"

#include "FlecsGameLoopPhaseTree.generated.h"

class UFlecsWorld;

UNREALFLECS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Flecs_Phases);
UNREALFLECS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Flecs_Phases_OnLoad);
UNREALFLECS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Flecs_Phases_PostLoad);
UNREALFLECS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Flecs_Phases_PreUpdate);
UNREALFLECS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Flecs_Phases_OnUpdate);
UNREALFLECS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Flecs_Phases_OnValidate);
UNREALFLECS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Flecs_Phases_PostUpdate);
UNREALFLECS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Flecs_Phases_PreStore);
UNREALFLECS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Flecs_Phases_OnStore);

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsGameLoopPhaseNode
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category="Flecs|Game Loop", meta=(Categories="Flecs.Phases"))
	FGameplayTag PhaseTag;

	UPROPERTY(EditAnywhere, Category="Flecs|Game Loop")
	TArray<TInstancedStruct<FFlecsGameLoopPhaseNode>> ChildPhases;

	void CreatePhasesRecursively(const TSolidNotNull<UFlecsWorld*> InFlecsWorld, const FFlecsId ParentPhaseEntity) const;

#if WITH_EDITOR
	EDataValidationResult IsDataValid(FDataValidationContext& Context) const;
#endif // WITH_EDITOR
};

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsGameLoopPhaseTreeEntry
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category="Flecs|Game Loop")
	TArray<FFlecsGameLoopPhaseNode> Phases;
	
}; // struct FFlecsGameLoopPhaseTreeEntry

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsGameLoopPhaseTree
{
	GENERATED_BODY()

public:
	FFlecsGameLoopPhaseTree();
	
	UPROPERTY(EditAnywhere, Category="Flecs|Game Loop", meta=(ShowOnlyInnerProperties))
	TMap<EFlecsDefaultPipelinePhase, FFlecsGameLoopPhaseTreeEntry> DefaultPhases;
	
	UPROPERTY(EditAnywhere, Category="Flecs|Game Loop")
	TArray<FFlecsGameLoopPhaseNode> AdditionalPhases;

	void CreatePhasesFromTree(const TSolidNotNull<UFlecsWorld*> InFlecsWorld) const;

#if WITH_EDITOR

	EDataValidationResult IsDataValid(FDataValidationContext& Context) const;

#endif // WITH_EDITOR
	
}; // struct FFlecsGameLoopPhaseTree