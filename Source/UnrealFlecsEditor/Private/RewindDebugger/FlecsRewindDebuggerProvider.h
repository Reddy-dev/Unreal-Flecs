// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Common/ProviderLock.h"
#include "Model/PointTimeline.h"
#include "SolidMacros/Macros.h"
#include "TraceServices/Model/AnalysisSession.h"

namespace UE::Flecs::RewindDebugger
{
	struct FEntityState
	{
		FString Snapshot;
	};

	struct FEntityTimeline
	{
		FEntityTimeline(
			TraceServices::IAnalysisSession& InSession,
			const uint64 InWorldId,
			const uint64 InEntityId,
			const double InBeginTime)
			: WorldId(InWorldId)
			, EntityId(InEntityId)
			, BeginTime(InBeginTime)
			, States(MakeShared<TraceServices::TPointTimeline<FEntityState>>(InSession.GetLinearAllocator()))
		{
		}

		uint64 WorldId = 0;
		uint64 EntityId = 0;
		double BeginTime = 0.0;
		TOptional<double> EndTime;
		TSharedRef<TraceServices::TPointTimeline<FEntityState>> States;
	};

	class FProvider final : public TraceServices::IProvider, public TraceServices::IEditableProvider
	{
	public:
		explicit FProvider(TraceServices::IAnalysisSession& InSession);

		virtual void BeginRead() const override;
		virtual void EndRead() const override;
		virtual void ReadAccessCheck() const override;

		virtual void BeginEdit() const override;
		virtual void EndEdit() const override;
		virtual void EditAccessCheck() const override;

		void AppendState(
			uint64 InWorldId,
			uint64 InEntityId,
			double InTime,
			FString&& InSnapshot);
		void EndEntity(uint64 InWorldId, uint64 InEntityId, double InTime);

		bool HasWorld(uint64 InWorldId) const;
		void EnumerateEntities(
			uint64 InWorldId,
			TFunctionRef<void(const FEntityTimeline&)> InCallback) const;
		bool ReadStateAtTime(
			uint64 InWorldId,
			uint64 InEntityId,
			double InTime,
			FEntityState& OutState) const;

		static const FName ProviderName;

	private:
		using FEntityLifetimes = TArray<TSharedRef<FEntityTimeline>>;
		using FEntityMap = TMap<uint64, FEntityLifetimes>;

		TSharedRef<FEntityTimeline>* FindOrAddTimeline(
			uint64 InWorldId,
			uint64 InEntityId,
			double InTime);

		TraceServices::IAnalysisSession& Session;
		TMap<uint64, FEntityMap> Worlds;
		mutable TraceServices::FProviderLock Lock;
	};
} // namespace UE::Flecs::RewindDebugger
