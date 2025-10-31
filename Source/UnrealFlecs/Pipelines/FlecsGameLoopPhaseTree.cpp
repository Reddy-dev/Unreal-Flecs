// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "FlecsGameLoopPhaseTree.h"

#include "UnrealFlecsPhaseTag.h"
#include "Misc/DataValidation.h"

#include "Worlds/FlecsWorld.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsGameLoopPhaseTree)

UE_DEFINE_GAMEPLAY_TAG(Flecs_Phases_OnLoad, "Flecs.Phases.OnLoad")
UE_DEFINE_GAMEPLAY_TAG(Flecs_Phases_PostLoad, "Flecs.Phases.PostLoad")
UE_DEFINE_GAMEPLAY_TAG(Flecs_Phases_PreUpdate, "Flecs.Phases.PreUpdate")
UE_DEFINE_GAMEPLAY_TAG(Flecs_Phases_OnUpdate, "Flecs.Phases.OnUpdate")
UE_DEFINE_GAMEPLAY_TAG(Flecs_Phases_OnValidate, "Flecs.Phases.OnValidate")
UE_DEFINE_GAMEPLAY_TAG(Flecs_Phases_PostUpdate, "Flecs.Phases.PostUpdate")
UE_DEFINE_GAMEPLAY_TAG(Flecs_Phases_PreStore, "Flecs.Phases.PreStore")
UE_DEFINE_GAMEPLAY_TAG(Flecs_Phases_OnStore, "Flecs.Phases.OnStore")

void FFlecsGameLoopPhaseNode::CreatePhasesRecursively(const TSolidNotNull<UFlecsWorld*> InFlecsWorld,
	const FFlecsId ParentPhaseEntity) const
{
	solid_checkf(PhaseTag.IsValid(),
		TEXT("FFlecsGameLoopPhaseNode::CreatePhasesRecursively: Phase Tag is invalid"));
	
	const FFlecsEntityHandle PhaseEntity = InFlecsWorld->CreateEntity(PhaseTag.ToString())
		.Add(flecs::Phase)
		.Add<FUnrealFlecsPhaseTag>()
		.Set<FGameplayTag>(PhaseTag);

	if (ParentPhaseEntity.IsValid())
	{
		PhaseEntity.AddPair(flecs::DependsOn, ParentPhaseEntity);
	}
	
	solid_check(PhaseEntity.IsValid());

	for (const TInstancedStruct<FFlecsGameLoopPhaseNode>& ChildPhaseNode : ChildPhases)
	{
		solid_checkf(ChildPhaseNode.IsValid(),
			TEXT("FFlecsGameLoopPhaseNode::CreatePhasesRecursively: Child Phase Node is invalid for phase '%s'"),
			*PhaseTag.ToString());
		
		ChildPhaseNode.Get<FFlecsGameLoopPhaseNode>().CreatePhasesRecursively(InFlecsWorld, PhaseEntity);
	}
}

#if WITH_EDITOR

EDataValidationResult FFlecsGameLoopPhaseNode::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = EDataValidationResult::Valid;

	if (!PhaseTag.IsValid())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(FText::FromString(TEXT("Phase Node has an empty Phase Name.")));
		return Result;
	}

	for (const TInstancedStruct<FFlecsGameLoopPhaseNode>& ChildPhaseNode : ChildPhases)
	{
		if (!ChildPhaseNode.IsValid())
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::FromString(FString::Printf(
				TEXT("Child Phase Node is invalid for phase '%s'."), *PhaseTag.ToString())));
			continue;
		}

		const EDataValidationResult ChildResult
			= ChildPhaseNode.Get<FFlecsGameLoopPhaseNode>().IsDataValid(Context);

		if (ChildResult == EDataValidationResult::Invalid)
		{
			Result = EDataValidationResult::Invalid;
		}
	}

	return Result;
}

#endif // WITH_EDITOR

FFlecsGameLoopPhaseTree::FFlecsGameLoopPhaseTree()
{
	DefaultPhases.Add(EFlecsDefaultPipelinePhase::OnLoad, FFlecsGameLoopPhaseTreeEntry());
	DefaultPhases.Add(EFlecsDefaultPipelinePhase::PostLoad, FFlecsGameLoopPhaseTreeEntry());
	DefaultPhases.Add(EFlecsDefaultPipelinePhase::PreUpdate, FFlecsGameLoopPhaseTreeEntry());
	DefaultPhases.Add(EFlecsDefaultPipelinePhase::OnUpdate, FFlecsGameLoopPhaseTreeEntry());
	DefaultPhases.Add(EFlecsDefaultPipelinePhase::OnValidate, FFlecsGameLoopPhaseTreeEntry());
	DefaultPhases.Add(EFlecsDefaultPipelinePhase::PostUpdate, FFlecsGameLoopPhaseTreeEntry());
	DefaultPhases.Add(EFlecsDefaultPipelinePhase::PreStore, FFlecsGameLoopPhaseTreeEntry());
	DefaultPhases.Add(EFlecsDefaultPipelinePhase::OnStore, FFlecsGameLoopPhaseTreeEntry());
}

void FFlecsGameLoopPhaseTree::CreatePhasesFromTree(const TSolidNotNull<UFlecsWorld*> InFlecsWorld) const
{
	for (const TTuple<EFlecsDefaultPipelinePhase, FFlecsGameLoopPhaseTreeEntry>& PhaseNode : DefaultPhases)
	{
		const EFlecsDefaultPipelinePhase Phase = PhaseNode.Key;

		const FFlecsId PhaseId = ConvertDefaultPipelinePhaseToFlecsId(Phase);

		for (const FFlecsGameLoopPhaseNode& PhaseNodeInstance : PhaseNode.Value.Phases)
		{
			PhaseNodeInstance.CreatePhasesRecursively(InFlecsWorld, PhaseId);
		}
	}

	for (const FFlecsGameLoopPhaseNode& RootPhaseNode : AdditionalPhases)
	{
		RootPhaseNode.CreatePhasesRecursively(InFlecsWorld, FFlecsId());
	}
}

#if WITH_EDITOR

EDataValidationResult FFlecsGameLoopPhaseTree::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = EDataValidationResult::Valid;

	for (const TTuple<EFlecsDefaultPipelinePhase, FFlecsGameLoopPhaseTreeEntry>& PhaseNode : DefaultPhases)
	{
		for (const FFlecsGameLoopPhaseNode& PhaseNodeInstance : PhaseNode.Value.Phases)
		{
			if (!PhaseNodeInstance.PhaseTag.IsValid())
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::FromString(TEXT("Phase Node has an empty Phase Name.")));
			}
			else
			{
				const EDataValidationResult ChildResult = PhaseNodeInstance.IsDataValid(Context);

				Result = CombineDataValidationResults(Result, ChildResult);
			}
		}
	}

	return Result;
}

#endif // WITH_EDITOR
