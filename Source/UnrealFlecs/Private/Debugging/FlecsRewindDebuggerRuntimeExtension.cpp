// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Debugging/FlecsRewindDebuggerRuntimeExtension.h"

#include "Debugging/FlecsRewindDebuggerTag.h"
#include "Engine/World.h"
#include "General/FlecsEditorPerProjectDeveloperSettings.h"
#include "ObjectTrace.h"
#include "Serialization/JsonSerializer.h"
#include "Trace/Trace.h"
#include "Worlds/FlecsWorld.h"
#include "Worlds/FlecsWorldSubsystem.h"

UE_TRACE_CHANNEL_DEFINE(FlecsChannel);

UE_TRACE_EVENT_BEGIN(Flecs, EntityState)
	UE_TRACE_EVENT_FIELD(uint64, Cycle)
	UE_TRACE_EVENT_FIELD(uint64, WorldId)
	UE_TRACE_EVENT_FIELD(uint64, EntityId)
	UE_TRACE_EVENT_FIELD(UE::Trace::WideString, Snapshot)
UE_TRACE_EVENT_END()

UE_TRACE_EVENT_BEGIN(Flecs, EntityEnd)
	UE_TRACE_EVENT_FIELD(uint64, Cycle)
	UE_TRACE_EVENT_FIELD(uint64, WorldId)
	UE_TRACE_EVENT_FIELD(uint64, EntityId)
UE_TRACE_EVENT_END()

namespace UE::Flecs::RewindDebugger::Private
{
	constexpr int32 SnapshotSchemaVersion = 1;

	FString IdToString(const ecs_world_t* InWorld, const ecs_id_t InId)
	{
		char* IdString = ecs_id_str(InWorld, InId);
		if (!IdString)
		{
			return FString::Printf(TEXT("%llu"), static_cast<uint64>(InId));
		}

		const FString Result = UTF8_TO_TCHAR(IdString);
		ecs_os_free(IdString);
		return Result;
	}

	FString EntityPath(const ecs_world_t* InWorld, const ecs_entity_t InEntity)
	{
		char* Path = ecs_get_path_w_sep(InWorld, 0, InEntity, ".", nullptr);
		if (!Path)
		{
			return FString();
		}

		const FString Result = UTF8_TO_TCHAR(Path);
		ecs_os_free(Path);
		return Result;
	}

	bool IsBuiltinBookkeepingId(const ecs_id_t InId)
	{
		return ECS_IS_PAIR(InId) && ECS_PAIR_FIRST(InId) == ecs_id(EcsIdentifier);
	}

	TSharedPtr<FJsonValue> TrySerializeValue(
		const ecs_world_t* InWorld,
		const ecs_entity_t InEntity,
		const ecs_id_t InId)
	{
		const ecs_entity_t TypeId = ecs_get_typeid(InWorld, InId);
		if (TypeId == 0)
		{
			return nullptr;
		}

		const void* Value = ecs_get_id(InWorld, InEntity, InId);
		if (!Value)
		{
			return nullptr;
		}

		char* Json = ecs_ptr_to_json(InWorld, TypeId, Value);
		if (!Json)
		{
			return nullptr;
		}

		const FString JsonString = UTF8_TO_TCHAR(Json);
		ecs_os_free(Json);

		TSharedPtr<FJsonValue> JsonValue;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
		if (!FJsonSerializer::Deserialize(Reader, JsonValue) || !JsonValue.IsValid())
		{
			return nullptr;
		}

		return JsonValue;
	}

	TSharedRef<FJsonObject> BuildMember(
		const ecs_world_t* InWorld,
		const ecs_entity_t InEntity,
		const ecs_id_t InId)
	{
		TSharedRef<FJsonObject> Member = MakeShared<FJsonObject>();
		Member->SetStringField(TEXT("id"), LexToString(static_cast<uint64>(InId)));
		Member->SetStringField(TEXT("name"), IdToString(InWorld, InId));

		if (ECS_IS_PAIR(InId))
		{
			const ecs_entity_t Relation = ECS_PAIR_FIRST(InId);
			const ecs_entity_t Target = ECS_PAIR_SECOND(InId);

			Member->SetStringField(TEXT("kind"), TEXT("pair"));
			Member->SetStringField(TEXT("relation"), IdToString(InWorld, Relation));
			Member->SetStringField(TEXT("target"), IdToString(InWorld, Target));
		}
		else if (ecs_get_typeid(InWorld, InId) == 0)
		{
			Member->SetStringField(TEXT("kind"), TEXT("tag"));
		}
		else
		{
			Member->SetStringField(TEXT("kind"), TEXT("component"));
		}

		if (const TSharedPtr<FJsonValue> Value = TrySerializeValue(InWorld, InEntity, InId))
		{
			Member->SetBoolField(TEXT("valueAvailable"), true);
			Member->SetField(TEXT("value"), Value);
		}
		else
		{
			Member->SetBoolField(TEXT("valueAvailable"), false);
		}

		return Member;
	}

	FString BuildSnapshot(const flecs::world& InWorld, const flecs::entity_t InEntity)
	{
		const ecs_world_t* NativeWorld = InWorld.c_ptr();
		const ecs_type_t* EntityType = ecs_get_type(NativeWorld, InEntity);

		TArray<TSharedPtr<FJsonValue>> Members;
		if (EntityType)
		{
			Members.Reserve(EntityType->count);
			for (int32 Index = 0; Index < EntityType->count; ++Index)
			{
				const ecs_id_t Id = EntityType->array[Index];
				if (!IsBuiltinBookkeepingId(Id))
				{
					Members.Add(MakeShared<FJsonValueObject>(BuildMember(NativeWorld, InEntity, Id)));
				}
			}
		}

		const char* Name = ecs_get_name(NativeWorld, InEntity);

		TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
		Root->SetNumberField(TEXT("schemaVersion"), SnapshotSchemaVersion);
		Root->SetStringField(TEXT("entityId"), LexToString(static_cast<uint64>(InEntity)));
		Root->SetNumberField(TEXT("generation"), flecs::get_generation(InEntity));
		Root->SetStringField(TEXT("name"), Name ? UTF8_TO_TCHAR(Name) : FString());
		Root->SetStringField(TEXT("path"), EntityPath(NativeWorld, InEntity));
		Root->SetArrayField(TEXT("members"), MoveTemp(Members));

		FString Snapshot;
		const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Snapshot);
		FJsonSerializer::Serialize(Root, Writer);
		return Snapshot;
	}

	void TraceEntityEnd(const uint64 InWorldId, const uint64 InEntityId)
	{
		UE_TRACE_LOG(Flecs, EntityEnd, FlecsChannel)
			<< EntityEnd.Cycle(FPlatformTime::Cycles64())
			<< EntityEnd.WorldId(InWorldId)
			<< EntityEnd.EntityId(InEntityId);
	}
} // namespace UE::Flecs::RewindDebugger::Private

FFlecsRewindDebuggerRuntimeExtension::FFlecsRewindDebuggerRuntimeExtension()
{
	UE::Trace::ToggleChannel(TEXT("Flecs"), false);

	WorldPostActorTickHandle = FWorldDelegates::OnWorldPostActorTick.AddRaw(
		this,
		&FFlecsRewindDebuggerRuntimeExtension::HandleWorldPostActorTick);
	WorldCleanupHandle = FWorldDelegates::OnWorldCleanup.AddRaw(
		this,
		&FFlecsRewindDebuggerRuntimeExtension::HandleWorldCleanup);
}

FFlecsRewindDebuggerRuntimeExtension::~FFlecsRewindDebuggerRuntimeExtension()
{
	FWorldDelegates::OnWorldPostActorTick.Remove(WorldPostActorTickHandle);
	FWorldDelegates::OnWorldCleanup.Remove(WorldCleanupHandle);

	bCaptureEnabled.Store(false);
	UE::Trace::ToggleChannel(TEXT("Flecs"), false);
	ResetCaptureState();
}

void FFlecsRewindDebuggerRuntimeExtension::RecordingStarted()
{
	if (bEnableNextRecording.Load())
	{
		UE::Trace::ToggleChannel(TEXT("Flecs"), true);
		bCaptureEnabled.Store(true);
	}
}

void FFlecsRewindDebuggerRuntimeExtension::RecordingStopped()
{
	bCaptureEnabled.Store(false);
	UE::Trace::ToggleChannel(TEXT("Flecs"), false);
	ResetCaptureState();
}

void FFlecsRewindDebuggerRuntimeExtension::Clear()
{
	bCaptureEnabled.Store(false);
	UE::Trace::ToggleChannel(TEXT("Flecs"), false);
	ResetCaptureState();

	const UFlecsEditorPerProjectDeveloperSettings* Settings =
		GetDefault<UFlecsEditorPerProjectDeveloperSettings>();
	bEnableNextRecording.Store(Settings && Settings->bEnableFlecsRewindDebugger);
}

void FFlecsRewindDebuggerRuntimeExtension::HandleWorldPostActorTick(
	UWorld* InWorld,
	MAYBE_UNUSED ELevelTick InTickType,
	MAYBE_UNUSED float InDeltaSeconds)
{
	if (!bCaptureEnabled.Load() || !IsValid(InWorld)
		|| (InWorld->WorldType != EWorldType::PIE && InWorld->WorldType != EWorldType::Game))
	{
		return;
	}

	UFlecsWorldSubsystem* Subsystem = InWorld->GetSubsystem<UFlecsWorldSubsystem>();
	if (!IsValid(Subsystem) || !Subsystem->HasValidFlecsWorld())
	{
		if (const uint64* EndedWorldId = WorldIds.Find(InWorld))
		{
			EndWorldEntities(*EndedWorldId);
			WorldIds.Remove(InWorld);
		}
		return;
	}

	UFlecsWorld* FlecsWorld = Subsystem->GetDefaultWorld();
	if (!IsValid(FlecsWorld) || !FlecsWorld->bIsInitialized)
	{
		if (const uint64* EndedWorldId = WorldIds.Find(InWorld))
		{
			EndWorldEntities(*EndedWorldId);
			WorldIds.Remove(InWorld);
		}
		return;
	}

	TRACE_OBJECT(FlecsWorld);
	const uint64 WorldId = FObjectTrace::GetObjectId(FlecsWorld);
	if (WorldId == 0)
	{
		return;
	}

	uint64& PreviousWorldId = WorldIds.FindOrAdd(InWorld);
	if (PreviousWorldId != 0 && PreviousWorldId != WorldId)
	{
		EndWorldEntities(PreviousWorldId);
	}
	PreviousWorldId = WorldId;

	const flecs::world NativeWorld = FlecsWorld->GetNativeFlecsWorld();
	const flecs::id_t DebugTagId = NativeWorld.id_if_registered<FFlecsRewindDebuggerTag>();
	TSet<FEntityKey> SeenEntities;

	if (DebugTagId != 0)
	{
		ecs_iter_t Iterator = ecs_each_id(NativeWorld.c_ptr(), DebugTagId);
		while (ecs_each_next(&Iterator))
		{
			for (int32 Index = 0; Index < Iterator.count; ++Index)
			{
				const uint64 EntityId = Iterator.entities[Index];
				const FEntityKey Key { WorldId, EntityId };
				SeenEntities.Add(Key);

				const FString Snapshot =
					UE::Flecs::RewindDebugger::Private::BuildSnapshot(NativeWorld, EntityId);
				const FString* PreviousSnapshot = PreviousSnapshots.Find(Key);
				if (!PreviousSnapshot || *PreviousSnapshot != Snapshot)
				{
					UE_TRACE_LOG(Flecs, EntityState, FlecsChannel)
						<< EntityState.Cycle(FPlatformTime::Cycles64())
						<< EntityState.WorldId(WorldId)
						<< EntityState.EntityId(EntityId)
						<< EntityState.Snapshot(*Snapshot, Snapshot.Len());

					PreviousSnapshots.FindOrAdd(Key) = Snapshot;
				}
			}
		}
	}

	TArray<FEntityKey> EndedEntities;
	for (const TPair<FEntityKey, FString>& Entry : PreviousSnapshots)
	{
		if (Entry.Key.WorldId == WorldId && !SeenEntities.Contains(Entry.Key))
		{
			EndedEntities.Add(Entry.Key);
		}
	}

	for (const FEntityKey& Key : EndedEntities)
	{
		UE::Flecs::RewindDebugger::Private::TraceEntityEnd(Key.WorldId, Key.EntityId);
		PreviousSnapshots.Remove(Key);
	}
}

void FFlecsRewindDebuggerRuntimeExtension::HandleWorldCleanup(
	UWorld* InWorld,
	MAYBE_UNUSED bool bInSessionEnded,
	MAYBE_UNUSED bool bInCleanupResources)
{
	if (const uint64* WorldId = WorldIds.Find(InWorld))
	{
		if (bCaptureEnabled.Load())
		{
			EndWorldEntities(*WorldId);
		}
		WorldIds.Remove(InWorld);
	}
}

void FFlecsRewindDebuggerRuntimeExtension::EndWorldEntities(const uint64 InWorldId)
{
	TArray<FEntityKey> EndedEntities;
	for (const TPair<FEntityKey, FString>& Entry : PreviousSnapshots)
	{
		if (Entry.Key.WorldId == InWorldId)
		{
			EndedEntities.Add(Entry.Key);
		}
	}

	for (const FEntityKey& Key : EndedEntities)
	{
		UE::Flecs::RewindDebugger::Private::TraceEntityEnd(Key.WorldId, Key.EntityId);
		PreviousSnapshots.Remove(Key);
	}
}

void FFlecsRewindDebuggerRuntimeExtension::ResetCaptureState()
{
	PreviousSnapshots.Reset();
	WorldIds.Reset();
}
