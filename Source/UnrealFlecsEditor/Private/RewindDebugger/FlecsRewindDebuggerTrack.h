// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "IRewindDebuggerTrackCreator.h"
#include "RewindDebuggerTrack.h"
#include "SEventTimelineView.h"
#include "SolidMacros/Macros.h"

namespace UE::Flecs::RewindDebugger
{
	class FEntityTrack final : public ::RewindDebugger::FRewindDebuggerTrack
	{
	public:
		FEntityTrack(uint64 InWorldId, uint64 InEntityId);

		uint64 GetEntityId() const
		{
			return EntityId;
		}

	private:
		virtual bool UpdateInternal() override;
		virtual TSharedPtr<SWidget> GetTimelineViewInternal() override;
		virtual TSharedPtr<SWidget> GetDetailsViewInternal() override;
		virtual FName GetNameInternal() const override;
		virtual FText GetDisplayNameInternal() const override;
		virtual uint64 GetObjectIdInternal() const override;

		TSharedPtr<SEventTimelineView::FTimelineEventData> GetEventData() const;
		void RefreshEventData() const;

		uint64 WorldId = 0;
		uint64 EntityId = 0;
		FText DisplayName;
		mutable TSharedPtr<SEventTimelineView::FTimelineEventData> EventData;
	};

	class FEntitiesTrack final : public ::RewindDebugger::FRewindDebuggerTrack
	{
	public:
		explicit FEntitiesTrack(uint64 InWorldId);

	private:
		virtual bool UpdateInternal() override;
		virtual FName GetNameInternal() const override;
		virtual FText GetDisplayNameInternal() const override;
		virtual uint64 GetObjectIdInternal() const override;
		virtual TConstArrayView<TSharedPtr<::RewindDebugger::FRewindDebuggerTrack>>
			GetChildrenInternal(
				TArray<TSharedPtr<::RewindDebugger::FRewindDebuggerTrack>>& OutTracks) const override;

		uint64 WorldId = 0;
		TArray<TSharedPtr<FEntityTrack>> Children;
	};

	class FTrackCreator final : public ::RewindDebugger::IRewindDebuggerTrackCreator
	{
	private:
		virtual FName GetTargetTypeNameInternal() const override;
		virtual FName GetNameInternal() const override;
		virtual void GetTrackTypesInternal(
			TArray<::RewindDebugger::FRewindDebuggerTrackType>& OutTypes) const override;
		virtual bool HasDebugInfoInternal(
			const ::RewindDebugger::FObjectId& InObjectId) const override;
		virtual TSharedPtr<::RewindDebugger::FRewindDebuggerTrack> CreateTrackInternal(
			const ::RewindDebugger::FObjectId& InObjectId) const override;
	};
} // namespace UE::Flecs::RewindDebugger
