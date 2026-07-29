// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "RewindDebugger/FlecsRewindDebuggerTrack.h"

#include "Common/ProviderLock.h"
#include "Dom/JsonObject.h"
#include "IRewindDebugger.h"
#include "RewindDebugger/FlecsRewindDebuggerProvider.h"
#include "Serialization/JsonSerializer.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Worlds/FlecsWorld.h"

#define LOCTEXT_NAMESPACE "FlecsRewindDebugger"

namespace UE::Flecs::RewindDebugger
{
	namespace Private
	{
		const FName TrackName(TEXT("FlecsEntities"));

		const FProvider* GetProvider()
		{
			const IRewindDebugger* Debugger = IRewindDebugger::Instance();
			const TraceServices::IAnalysisSession* Session =
				Debugger ? Debugger->GetAnalysisSession() : nullptr;
			return Session ? Session->ReadProvider<FProvider>(FProvider::ProviderName) : nullptr;
		}

		TSharedPtr<FJsonObject> ParseSnapshot(const FString& InSnapshot)
		{
			TSharedPtr<FJsonObject> Root;
			const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(InSnapshot);
			if (!FJsonSerializer::Deserialize(Reader, Root))
			{
				return nullptr;
			}
			return Root;
		}

		FString JsonValueToString(const TSharedPtr<FJsonValue>& InValue)
		{
			if (!InValue.IsValid())
			{
				return TEXT("Unavailable");
			}

			FString Result;
			const TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> Writer =
				TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&Result);
			FJsonSerializer::Serialize(InValue, FString(), Writer);
			return Result;
		}

		class SEntityDetails final : public SCompoundWidget
		{
		public:
			SLATE_BEGIN_ARGS(SEntityDetails)
			{
			}
				SLATE_ARGUMENT(uint64, WorldId)
				SLATE_ARGUMENT(uint64, EntityId)
			SLATE_END_ARGS()

			void Construct(const FArguments& InArguments)
			{
				WorldId = InArguments._WorldId;
				EntityId = InArguments._EntityId;

				ChildSlot
				[
					SAssignNew(Content, SScrollBox)
				];
				Refresh();
			}

			virtual void Tick(
				const FGeometry& InAllottedGeometry,
				const double InCurrentTime,
				const float InDeltaTime) override
			{
				SCompoundWidget::Tick(InAllottedGeometry, InCurrentTime, InDeltaTime);
				Refresh();
			}

		private:
			void AddField(const FString& InLabel, const FString& InValue) const
			{
				Content->AddSlot()
				.Padding(4.0f, 2.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(FString::Printf(
						TEXT("%s: %s"),
						*InLabel,
						*InValue)))
					.AutoWrapText(true)
				];
			}

			void Refresh()
			{
				const FProvider* Provider = GetProvider();
				const IRewindDebugger* Debugger = IRewindDebugger::Instance();
				if (!Provider || !Debugger)
				{
					return;
				}

				FEntityState State;
				{
					TraceServices::FProviderReadScopeLock ProviderReadScope(*Provider);
					if (!Provider->ReadStateAtTime(
						WorldId,
						EntityId,
						Debugger->CurrentTraceTime(),
						State))
					{
						if (!bShowingUnavailable)
						{
							bShowingUnavailable = true;
							LastSnapshot.Reset();
							Content->ClearChildren();
							AddField(TEXT("State"), TEXT("Entity is not alive at the current scrub time"));
						}
						return;
					}
				}

				if (State.Snapshot == LastSnapshot)
				{
					return;
				}

				const TSharedPtr<FJsonObject> Root = ParseSnapshot(State.Snapshot);
				if (!Root)
				{
					return;
				}

				LastSnapshot = State.Snapshot;
				bShowingUnavailable = false;
				Content->ClearChildren();

				FString EntityIdString;
				FString Name;
				FString Path;
				double Generation = 0.0;
				Root->TryGetStringField(TEXT("entityId"), EntityIdString);
				Root->TryGetStringField(TEXT("name"), Name);
				Root->TryGetStringField(TEXT("path"), Path);
				Root->TryGetNumberField(TEXT("generation"), Generation);

				AddField(TEXT("Entity ID"), EntityIdString);
				AddField(TEXT("Generation"), LexToString(static_cast<uint32>(Generation)));
				AddField(TEXT("Historical Name"), Name);
				AddField(TEXT("Historical Path"), Path);

				Content->AddSlot()
					.Padding(4.0f)
					[
						SNew(SSeparator)
					];

				const TArray<TSharedPtr<FJsonValue>>* Members = nullptr;
				if (!Root->TryGetArrayField(TEXT("members"), Members))
				{
					return;
				}

				for (const TSharedPtr<FJsonValue>& MemberValue : *Members)
				{
					const TSharedPtr<FJsonObject>* Member = nullptr;
					if (!MemberValue->TryGetObject(Member) || !Member || !Member->IsValid())
					{
						continue;
					}

					FString MemberName;
					FString Kind;
					FString Relation;
					FString Target;
					bool bValueAvailable = false;
					(*Member)->TryGetStringField(TEXT("name"), MemberName);
					(*Member)->TryGetStringField(TEXT("kind"), Kind);
					(*Member)->TryGetStringField(TEXT("relation"), Relation);
					(*Member)->TryGetStringField(TEXT("target"), Target);
					(*Member)->TryGetBoolField(TEXT("valueAvailable"), bValueAvailable);

					FString Heading = FString::Printf(
						TEXT("%s  [%s]"),
						*MemberName,
						*Kind);
					if (Kind == TEXT("pair"))
					{
						Heading += FString::Printf(TEXT("  %s -> %s"), *Relation, *Target);
					}

					const FString ValueString = bValueAvailable
						? JsonValueToString((*Member)->TryGetField(TEXT("value")))
						: TEXT("Value unavailable (no Flecs metadata serializer)");

					Content->AddSlot()
						.Padding(4.0f, 2.0f)
						[
							SNew(SExpandableArea)
							.InitiallyCollapsed(true)
							.HeaderContent()
							[
								SNew(STextBlock)
								.Text(FText::FromString(Heading))
							]
							.BodyContent()
							[
								SNew(STextBlock)
								.Text(FText::FromString(ValueString))
								.AutoWrapText(true)
							]
						];
				}
			}

			uint64 WorldId = 0;
			uint64 EntityId = 0;
			FString LastSnapshot;
			bool bShowingUnavailable = false;
			TSharedPtr<SScrollBox> Content;
		};
	} // namespace Private

	FEntityTrack::FEntityTrack(const uint64 InWorldId, const uint64 InEntityId)
		: WorldId(InWorldId)
		, EntityId(InEntityId)
		, DisplayName(FText::FromString(LexToString(InEntityId)))
		, EventData(MakeShared<SEventTimelineView::FTimelineEventData>())
	{
	}

	bool FEntityTrack::UpdateInternal()
	{
		const FProvider* Provider = Private::GetProvider();
		const IRewindDebugger* Debugger = IRewindDebugger::Instance();
		if (!Provider || !Debugger)
		{
			return false;
		}

		FEntityState State;
		{
			TraceServices::FProviderReadScopeLock ProviderReadScope(*Provider);
			if (!Provider->ReadStateAtTime(
				WorldId,
				EntityId,
				Debugger->CurrentTraceTime(),
				State))
			{
				return false;
			}
		}

		const TSharedPtr<FJsonObject> Root = Private::ParseSnapshot(State.Snapshot);
		if (!Root)
		{
			return false;
		}

		FString Name;
		double Generation = 0.0;
		Root->TryGetStringField(TEXT("name"), Name);
		Root->TryGetNumberField(TEXT("generation"), Generation);
		if (Name.IsEmpty())
		{
			Name = TEXT("<unnamed>");
		}

		const FText NewDisplayName = FText::FromString(FString::Printf(
			TEXT("%s [%llu:%u]"),
			*Name,
			EntityId & ECS_ENTITY_MASK,
			static_cast<uint32>(Generation)));
		const bool bChanged = !DisplayName.EqualTo(NewDisplayName);
		DisplayName = NewDisplayName;
		RefreshEventData();
		return bChanged;
	}

	TSharedPtr<SWidget> FEntityTrack::GetTimelineViewInternal()
	{
		return SNew(SEventTimelineView)
			.ViewRange_Lambda([]()
			{
				return IRewindDebugger::Instance()->GetCurrentViewRange();
			})
			.EventData_Raw(this, &FEntityTrack::GetEventData);
	}

	TSharedPtr<SWidget> FEntityTrack::GetDetailsViewInternal()
	{
		return SNew(Private::SEntityDetails)
			.WorldId(WorldId)
			.EntityId(EntityId);
	}

	FName FEntityTrack::GetNameInternal() const
	{
		return FName(*FString::Printf(TEXT("FlecsEntity_%llu"), EntityId));
	}

	FText FEntityTrack::GetDisplayNameInternal() const
	{
		return DisplayName;
	}

	uint64 FEntityTrack::GetObjectIdInternal() const
	{
		return WorldId;
	}

	TSharedPtr<SEventTimelineView::FTimelineEventData> FEntityTrack::GetEventData() const
	{
		RefreshEventData();
		return EventData;
	}

	void FEntityTrack::RefreshEventData() const
	{
		EventData->EventPoints.Reset();
		EventData->EventWindows.Reset();

		const FProvider* Provider = Private::GetProvider();
		const IRewindDebugger* Debugger = IRewindDebugger::Instance();
		if (!Provider || !Debugger)
		{
			return;
		}

		const TRange<double>& Range = Debugger->GetCurrentTraceRange();
		const double StartTime = Range.GetLowerBoundValue();
		const double EndTime = Range.GetUpperBoundValue();

		TraceServices::FProviderReadScopeLock ProviderReadScope(*Provider);
		Provider->EnumerateEntities(
			WorldId,
			[this, StartTime, EndTime, Debugger](const FEntityTimeline& InTimeline)
			{
				if (InTimeline.EntityId != EntityId)
				{
					return;
				}

				SEventTimelineView::FTimelineEventData::FEventWindow Lifetime;
				Lifetime.TimeStart = InTimeline.BeginTime;
				Lifetime.TimeEnd = InTimeline.EndTime.Get(
					Debugger->GetAnalysisSession()->GetDurationSeconds());
				Lifetime.Type = LOCTEXT("FlecsEntityLifetime", "Flecs Entity Lifetime");
				Lifetime.Description = DisplayName;
				Lifetime.Color = FLinearColor(0.12f, 0.55f, 0.25f, 0.5f);
				EventData->EventWindows.Add(MoveTemp(Lifetime));

				InTimeline.States->EnumerateEvents(
					StartTime,
					EndTime,
					[this](
						const double InStartTime,
						MAYBE_UNUSED double InEndTime,
						MAYBE_UNUSED uint32 InDepth,
						MAYBE_UNUSED const FEntityState& InState)
					{
						SEventTimelineView::FTimelineEventData::FEventPoint Point;
						Point.Time = InStartTime;
						Point.Type = LOCTEXT("FlecsEntityState", "Flecs State Change");
						Point.Description = DisplayName;
						Point.Color = FLinearColor(0.2f, 0.8f, 0.35f);
						EventData->EventPoints.Add(MoveTemp(Point));
						return TraceServices::EEventEnumerate::Continue;
					});
			});
	}

	FEntitiesTrack::FEntitiesTrack(const uint64 InWorldId)
		: WorldId(InWorldId)
	{
	}

	bool FEntitiesTrack::UpdateInternal()
	{
		const FProvider* Provider = Private::GetProvider();
		if (!Provider)
		{
			return false;
		}

		TArray<uint64> EntityIds;
		{
			TraceServices::FProviderReadScopeLock ProviderReadScope(*Provider);
			Provider->EnumerateEntities(
				WorldId,
				[&EntityIds](const FEntityTimeline& InTimeline)
				{
					EntityIds.AddUnique(InTimeline.EntityId);
				});
		}
		EntityIds.Sort();

		bool bChanged = Children.Num() != EntityIds.Num();
		TArray<TSharedPtr<FEntityTrack>> NewChildren;
		NewChildren.Reserve(EntityIds.Num());

		for (const uint64 EntityId : EntityIds)
		{
			TSharedPtr<FEntityTrack>* Existing = Children.FindByPredicate(
				[EntityId](const TSharedPtr<FEntityTrack>& InTrack)
				{
					return InTrack->GetEntityId() == EntityId;
				});

			TSharedPtr<FEntityTrack> Child = Existing
				? *Existing
				: MakeShared<FEntityTrack>(WorldId, EntityId);
			bChanged |= !Existing;
			bChanged |= Child->Update();
			NewChildren.Add(MoveTemp(Child));
		}

		Children = MoveTemp(NewChildren);
		return bChanged;
	}

	FName FEntitiesTrack::GetNameInternal() const
	{
		return Private::TrackName;
	}

	FText FEntitiesTrack::GetDisplayNameInternal() const
	{
		return LOCTEXT("FlecsEntitiesTrack", "Flecs Entities");
	}

	uint64 FEntitiesTrack::GetObjectIdInternal() const
	{
		return WorldId;
	}

	TConstArrayView<TSharedPtr<::RewindDebugger::FRewindDebuggerTrack>>
	FEntitiesTrack::GetChildrenInternal(
		TArray<TSharedPtr<::RewindDebugger::FRewindDebuggerTrack>>& OutTracks) const
	{
		for (const TSharedPtr<FEntityTrack>& Child : Children)
		{
			OutTracks.Add(Child);
		}
		return {};
	}

	FName FTrackCreator::GetTargetTypeNameInternal() const
	{
		return UFlecsWorld::StaticClass()->GetFName();
	}

	FName FTrackCreator::GetNameInternal() const
	{
		return Private::TrackName;
	}

	void FTrackCreator::GetTrackTypesInternal(
		TArray<::RewindDebugger::FRewindDebuggerTrackType>& OutTypes) const
	{
		OutTypes.Add({ Private::TrackName, LOCTEXT("FlecsEntitiesType", "Flecs Entities") });
	}

	bool FTrackCreator::HasDebugInfoInternal(
		const ::RewindDebugger::FObjectId& InObjectId) const
	{
		const FProvider* Provider = Private::GetProvider();
		if (!Provider)
		{
			return false;
		}

		TraceServices::FProviderReadScopeLock ProviderReadScope(*Provider);
		return Provider->HasWorld(InObjectId.GetMainId());
	}

	TSharedPtr<::RewindDebugger::FRewindDebuggerTrack> FTrackCreator::CreateTrackInternal(
		const ::RewindDebugger::FObjectId& InObjectId) const
	{
		return MakeShared<FEntitiesTrack>(InObjectId.GetMainId());
	}
} // namespace UE::Flecs::RewindDebugger

#undef LOCTEXT_NAMESPACE
