// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineBaseTypes.h"
#include "RewindDebuggerRuntimeInterface/IRewindDebuggerRuntimeExtension.h"
#include "SolidMacros/Macros.h"

class UWorld;

/**
 * Passive Rewind Debugger capture for explicitly tagged Flecs entities.
 */
class FFlecsRewindDebuggerRuntimeExtension final : public IRewindDebuggerRuntimeExtension
{
public:
	FFlecsRewindDebuggerRuntimeExtension();
	virtual ~FFlecsRewindDebuggerRuntimeExtension() override;

	virtual void RecordingStarted() override;
	virtual void RecordingStopped() override;
	virtual void Clear() override;

private:
	struct FEntityKey
	{
		uint64 WorldId = 0;
		uint64 EntityId = 0;

		friend bool operator==(const FEntityKey&, const FEntityKey&) = default;

		friend uint32 GetTypeHash(const FEntityKey& InKey)
		{
			return HashCombine(GetTypeHash(InKey.WorldId), GetTypeHash(InKey.EntityId));
		}
	};

	void HandleWorldPostActorTick(UWorld* InWorld, ELevelTick InTickType, float InDeltaSeconds);
	void HandleWorldCleanup(UWorld* InWorld, bool bInSessionEnded, bool bInCleanupResources);
	void EndWorldEntities(uint64 InWorldId);
	void ResetCaptureState();

	TAtomic<bool> bCaptureEnabled { false };
	TAtomic<bool> bEnableNextRecording { false };

	TMap<FEntityKey, FString> PreviousSnapshots;
	TMap<TWeakObjectPtr<UWorld>, uint64> WorldIds;

	FDelegateHandle WorldPostActorTickHandle;
	FDelegateHandle WorldCleanupHandle;
};
