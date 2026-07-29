// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "RewindDebugger/FlecsRewindDebuggerProvider.h"

namespace UE::Flecs::RewindDebugger
{
	thread_local TraceServices::FProviderLock::FThreadLocalState GFlecsRewindDebuggerProviderLockState;

	const FName FProvider::ProviderName(TEXT("FlecsRewindDebuggerProvider"));

	FProvider::FProvider(TraceServices::IAnalysisSession& InSession)
		: Session(InSession)
	{
	}

	void FProvider::BeginRead() const
	{
		Lock.BeginRead(GFlecsRewindDebuggerProviderLockState);
	}

	void FProvider::EndRead() const
	{
		Lock.EndRead(GFlecsRewindDebuggerProviderLockState);
	}

	void FProvider::ReadAccessCheck() const
	{
		Lock.ReadAccessCheck(GFlecsRewindDebuggerProviderLockState);
	}

	void FProvider::BeginEdit() const
	{
		Lock.BeginWrite(GFlecsRewindDebuggerProviderLockState);
	}

	void FProvider::EndEdit() const
	{
		Lock.EndWrite(GFlecsRewindDebuggerProviderLockState);
	}

	void FProvider::EditAccessCheck() const
	{
		Lock.WriteAccessCheck(GFlecsRewindDebuggerProviderLockState);
	}

	void FProvider::AppendState(
		const uint64 InWorldId,
		const uint64 InEntityId,
		const double InTime,
		FString&& InSnapshot)
	{
		EditAccessCheck();

		TSharedRef<FEntityTimeline>* Timeline =
			FindOrAddTimeline(InWorldId, InEntityId, InTime);
		(*Timeline)->States->AppendEvent(InTime, FEntityState { MoveTemp(InSnapshot) });

		TraceServices::FAnalysisSessionEditScope SessionEditScope(Session);
		Session.UpdateDurationSeconds(InTime);
	}

	void FProvider::EndEntity(
		const uint64 InWorldId,
		const uint64 InEntityId,
		const double InTime)
	{
		EditAccessCheck();

		FEntityMap* Entities = Worlds.Find(InWorldId);
		if (!Entities)
		{
			return;
		}

		FEntityLifetimes* Lifetimes = Entities->Find(InEntityId);
		if (!Lifetimes || Lifetimes->IsEmpty() || Lifetimes->Last()->EndTime.IsSet())
		{
			return;
		}

		Lifetimes->Last()->EndTime =
			FMath::Max(InTime, Lifetimes->Last()->BeginTime);

		TraceServices::FAnalysisSessionEditScope SessionEditScope(Session);
		Session.UpdateDurationSeconds(InTime);
	}

	bool FProvider::HasWorld(const uint64 InWorldId) const
	{
		ReadAccessCheck();
		const FEntityMap* Entities = Worlds.Find(InWorldId);
		return Entities && !Entities->IsEmpty();
	}

	void FProvider::EnumerateEntities(
		const uint64 InWorldId,
		TFunctionRef<void(const FEntityTimeline&)> InCallback) const
	{
		ReadAccessCheck();
		if (const FEntityMap* Entities = Worlds.Find(InWorldId))
		{
			for (const TPair<uint64, FEntityLifetimes>& Entry : *Entities)
			{
				for (const TSharedRef<FEntityTimeline>& Lifetime : Entry.Value)
				{
					InCallback(*Lifetime);
				}
			}
		}
	}

	bool FProvider::ReadStateAtTime(
		const uint64 InWorldId,
		const uint64 InEntityId,
		const double InTime,
		FEntityState& OutState) const
	{
		ReadAccessCheck();

		const FEntityMap* Entities = Worlds.Find(InWorldId);
		if (!Entities)
		{
			return false;
		}

		const FEntityLifetimes* Lifetimes = Entities->Find(InEntityId);
		if (!Lifetimes)
		{
			return false;
		}

		for (const TSharedRef<FEntityTimeline>& Timeline : *Lifetimes)
		{
			if (InTime < Timeline->BeginTime
				|| (Timeline->EndTime.IsSet() && InTime > Timeline->EndTime.GetValue()))
			{
				continue;
			}

			bool bFound = false;
			Timeline->States->EnumerateEvents(
				-TNumericLimits<double>::Max(),
				InTime,
				[&OutState, &bFound](
					MAYBE_UNUSED double InStartTime,
					MAYBE_UNUSED double InEndTime,
					MAYBE_UNUSED uint32 InDepth,
					const FEntityState& InState)
				{
					OutState = InState;
					bFound = true;
					return TraceServices::EEventEnumerate::Continue;
				});
			if (bFound)
			{
				return true;
			}
		}

		return false;
	}

	TSharedRef<FEntityTimeline>* FProvider::FindOrAddTimeline(
		const uint64 InWorldId,
		const uint64 InEntityId,
		const double InTime)
	{
		FEntityLifetimes& Lifetimes = Worlds.FindOrAdd(InWorldId).FindOrAdd(InEntityId);
		if (!Lifetimes.IsEmpty() && !Lifetimes.Last()->EndTime.IsSet())
		{
			return &Lifetimes.Last();
		}

		return &Lifetimes.Add_GetRef(
			MakeShared<FEntityTimeline>(Session, InWorldId, InEntityId, InTime));
	}
} // namespace UE::Flecs::RewindDebugger
