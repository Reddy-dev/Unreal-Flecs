// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "RewindDebugger/FlecsRewindDebuggerAnalyzer.h"

#include "Common/ProviderLock.h"
#include "Dom/JsonObject.h"
#include "RewindDebugger/FlecsRewindDebuggerProvider.h"
#include "Serialization/JsonSerializer.h"
#include "TraceServices/Model/AnalysisSession.h"

namespace UE::Flecs::RewindDebugger
{
	namespace Private
	{
		constexpr int32 SnapshotSchemaVersion = 1;
		constexpr int32 MaximumSnapshotCharacters = 4 * 1024 * 1024;
	}

	bool ValidateSnapshotPayload(const FString& InSnapshot, const uint64 InExpectedEntityId)
	{
		if (InSnapshot.IsEmpty() || InSnapshot.Len() > Private::MaximumSnapshotCharacters)
		{
			return false;
		}

		TSharedPtr<FJsonObject> Root;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(InSnapshot);
		if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
		{
			return false;
		}

		double SchemaVersion = 0.0;
		FString EntityIdString;
		FString EntityName;
		FString EntityPath;
		double Generation = 0.0;
		const TArray<TSharedPtr<FJsonValue>>* Members = nullptr;
		if (!Root->TryGetNumberField(TEXT("schemaVersion"), SchemaVersion)
			|| SchemaVersion != Private::SnapshotSchemaVersion
			|| !Root->TryGetStringField(TEXT("entityId"), EntityIdString)
			|| !Root->TryGetStringField(TEXT("name"), EntityName)
			|| !Root->TryGetStringField(TEXT("path"), EntityPath)
			|| !Root->TryGetNumberField(TEXT("generation"), Generation)
			|| Generation < 0.0
			|| Generation > TNumericLimits<uint32>::Max()
			|| FMath::FloorToDouble(Generation) != Generation
			|| !Root->TryGetArrayField(TEXT("members"), Members))
		{
			return false;
		}

		uint64 PayloadEntityId = 0;
		if (!LexTryParseString(PayloadEntityId, *EntityIdString)
			|| PayloadEntityId != InExpectedEntityId)
		{
			return false;
		}

		for (const TSharedPtr<FJsonValue>& MemberValue : *Members)
		{
			const TSharedPtr<FJsonObject>* Member = nullptr;
			if (!MemberValue.IsValid() || !MemberValue->TryGetObject(Member) || !Member || !Member->IsValid())
			{
				return false;
			}

			FString Id;
			FString MemberName;
			FString Kind;
			bool bValueAvailable = false;
			if (!(*Member)->TryGetStringField(TEXT("id"), Id)
				|| !(*Member)->TryGetStringField(TEXT("name"), MemberName)
				|| !(*Member)->TryGetStringField(TEXT("kind"), Kind)
				|| !(*Member)->TryGetBoolField(TEXT("valueAvailable"), bValueAvailable)
				|| (Kind != TEXT("component") && Kind != TEXT("tag") && Kind != TEXT("pair"))
				|| (bValueAvailable && !(*Member)->HasField(TEXT("value"))))
			{
				return false;
			}

			uint64 MemberId = 0;
			if (!LexTryParseString(MemberId, *Id))
			{
				return false;
			}

			if (Kind == TEXT("pair"))
			{
				FString Relation;
				FString Target;
				if (!(*Member)->TryGetStringField(TEXT("relation"), Relation)
					|| !(*Member)->TryGetStringField(TEXT("target"), Target))
				{
					return false;
				}
			}
		}

		return true;
	}

	FAnalyzer::FAnalyzer(
		TraceServices::IAnalysisSession& InSession,
		FProvider& InProvider)
		: Session(InSession)
		, Provider(InProvider)
	{
	}

	void FAnalyzer::OnAnalysisBegin(const FOnAnalysisContext& InContext)
	{
		FInterfaceBuilder& Builder = InContext.InterfaceBuilder;
		Builder.RouteEvent(RouteId_EntityState, "Flecs", "EntityState");
		Builder.RouteEvent(RouteId_EntityEnd, "Flecs", "EntityEnd");
	}

	bool FAnalyzer::OnEvent(
		const uint16 InRouteId,
		MAYBE_UNUSED EStyle InStyle,
		const FOnEventContext& InContext)
	{
		const FEventData& EventData = InContext.EventData;
		const uint64 Cycle = EventData.GetValue<uint64>("Cycle");
		const uint64 WorldId = EventData.GetValue<uint64>("WorldId");
		const uint64 EntityId = EventData.GetValue<uint64>("EntityId");
		const double Time = InContext.EventTime.AsSeconds(Cycle);

		TraceServices::FProviderEditScopeLock ProviderEditScope(Provider);

		switch (InRouteId)
		{
		case RouteId_EntityState:
		{
			FString Snapshot;
			if (EventData.GetString("Snapshot", Snapshot)
				&& ValidateSnapshotPayload(Snapshot, EntityId))
			{
				Provider.AppendState(WorldId, EntityId, Time, MoveTemp(Snapshot));
			}
			break;
		}
		case RouteId_EntityEnd:
			Provider.EndEntity(WorldId, EntityId, Time);
			break;
		default:
			return false;
		}

		return true;
	}
} // namespace UE::Flecs::RewindDebugger
