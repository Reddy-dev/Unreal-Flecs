// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"
#include "UnrealFlecsTests/Fixtures/FlecsReplicationFixture.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"

#include "Networking/FlecsComponentReplicationDescriptor.h"
#include "Networking/FlecsNetworkId.h"
#include "Networking/FlecsNetworkWorldSubsystem.h"
#include "Networking/FlecsReplicatedEntityComponent.h"
#include "Networking/FlecsReplicatedTrait.h"
#include "Networking/FlecsReplicationTypes.h"
#include "Networking/FlecsStablePathTag.h"

namespace UE::Flecs::Tests
{
	struct FNativeReplicatedValue
	{
		int32 Value = 0;
	};

	struct FNativeReplicatedValueDuplicate
	{
		int32 Value = 0;
	};

	struct FUnsupportedNativeReplicatedValue
	{
		int32 Value = 0;
	};

	class FTestReplicationRouter final : public IFlecsReplicationRouter
	{
	public:
		explicit FTestReplicationRouter(const FName& InRouteName)
			: RouteName(&InRouteName)
		{
		}

		virtual FFlecsReplicationRouteDescriptor Route(const FFlecsEntityHandle&) const override
		{
			FFlecsReplicationRouteDescriptor Result = FFlecsReplicationRouteDescriptor::Default();
			Result.LogicalKey = FFlecsReplicationRouteKey(*RouteName);
			return Result;
		}

	private:
		const FName* RouteName;
	}; // class FTestReplicationRouter

	class FTestFragmentInterestPolicy final
		: public TFlecsReplicationInterestPolicy<FFlecsReplicationEveryoneInterestDescriptor>
	{
	public:
		FTestFragmentInterestPolicy()
			: TFlecsReplicationInterestPolicy(TEXT("Tests.Fragment"))
		{
		}

	protected:
		virtual bool IsInterested(const FFlecsReplicationEveryoneInterestDescriptor&,
			const FFlecsReplicationInterestEvaluationQuery& Query) const override
		{
			const FFlecsReplicationTestLocalOnly* Fragment =
				Query.Context.Find<FFlecsReplicationTestLocalOnly>();
			return Fragment && Fragment->Value == 42;
		}
	}; // class FTestFragmentInterestPolicy

	class FTestInvalidInterestPolicy final
		: public TFlecsReplicationInterestPolicy<FFlecsReplicationTestInvalidInterestDescriptor>
	{
	public:
		FTestInvalidInterestPolicy()
			: TFlecsReplicationInterestPolicy(TEXT("Tests.InvalidReference"))
		{
		}

	protected:
		virtual bool IsInterested(const FFlecsReplicationTestInvalidInterestDescriptor&,
			const FFlecsReplicationInterestEvaluationQuery&) const override
		{
			return true;
		}
	}; // class FTestInvalidInterestPolicy

	template<typename T>
	FFlecsComponentHandle RegisterTestComponent(UFlecsWorld* World)
	{
		const FFlecsComponentHandle Component = World->RegisterComponentType<T>();
		const FFlecsComponentPropertiesDefinition Properties = FFlecsComponentPropertiesDefinition::Make<T>();
		Properties.PropertiesFunction(World, Component, Properties);
		return Component;
	}

	struct FCapturedReplicationEntity
	{
		FFlecsReplicationLayoutDefinition Layout;
		FFlecsReplicatedEntityUpdate Snapshot;
	};

	inline FCapturedReplicationEntity CaptureEntity(UFlecsNetworkWorldSubsystem* Subsystem,
		UFlecsReplicationCaptureTransport* Transport, const FFlecsEntityHandle& Entity)
	{
		Transport->Snapshots.Reset();
		const FFlecsNetworkId NetworkId = Subsystem->BeginReplicatingEntity(Entity);
		Subsystem->FlushServerReplicationForTesting();
		
		const FFlecsReplicatedEntityUpdate* FoundSnapshot = Transport->Snapshots.FindByPredicate(
			[NetworkId](const FFlecsReplicatedEntityUpdate& Candidate)
			{
				return Candidate.NetworkId == NetworkId;
			});
		check(FoundSnapshot);
		
		const FFlecsReplicatedEntityUpdate Snapshot = *FoundSnapshot;
		const FFlecsReplicationLayoutDefinition* Layout = Transport->Layouts.FindByPredicate(
			[&Snapshot](const FFlecsReplicationLayoutDefinition& Candidate)
			{
				return Candidate.LayoutId == Snapshot.LayoutId;
			});
		check(Layout);
		return { *Layout, Snapshot };
	}

	inline void EnqueueLayout(UFlecsNetworkWorldSubsystem* Subsystem, const FGuid& Shard,
		const FFlecsReplicationLayoutDefinition& Layout)
	{
		FFlecsReplicationInboxRecord Record;
		Record.Type = EFlecsReplicationInboxRecordType::Layout;
		Record.SourceShard = Shard;
		Record.Layout = Layout;
		Subsystem->EnqueueReceivedRecord(MoveTemp(Record));
	}

	inline void EnqueueSnapshot(UFlecsNetworkWorldSubsystem* Subsystem, const FGuid& Shard,
		const FFlecsReplicatedEntityUpdate& Snapshot)
	{
		FFlecsReplicationInboxRecord Record;
		Record.Type = EFlecsReplicationInboxRecordType::UpsertEntity;
		Record.SourceShard = Shard;
		Record.Update = Snapshot;
		Subsystem->EnqueueReceivedRecord(MoveTemp(Record));
	}

	inline void SetSnapshotValue(UFlecsWorld* World, FFlecsReplicatedEntityUpdate& Snapshot,
		const FFlecsReplicationLayoutDefinition& Layout, const FFlecsReplicationTestValue& Value)
	{
		const FFlecsComponentReplicationDescriptor* Descriptor =
			FFlecsComponentReplicationRegistry::Get(World).Find(World->GetIdIfRegistered<FFlecsReplicationTestValue>());
		check(Descriptor);
		
		for (FFlecsReplicatedValue& Serialized : Snapshot.Values)
		{
			if (Layout.Keys.IsValidIndex(Serialized.KeyIndex)
				&& Layout.Keys[Serialized.KeyIndex].TryGetStorageDescriptor(World) == Descriptor)
			{
				Serialized.Bytes.Reset();
				FMemoryWriter Writer(Serialized.Bytes, true);
				FFlecsReplicationTestValue Copy = Value;
				Descriptor->Serialize(Writer, &Copy);
				return;
			}
		}
	}

	inline void SetSnapshotPairValue(UFlecsWorld* World, FFlecsReplicatedEntityUpdate& Snapshot,
		const FFlecsReplicationLayoutDefinition& Layout, const FFlecsReplicationTestValueRelationship& Value)
	{
		const FFlecsComponentReplicationDescriptor* Descriptor =
			FFlecsComponentReplicationRegistry::Get(World).Find(
				World->GetIdIfRegistered<FFlecsReplicationTestValueRelationship>());
		check(Descriptor);

		for (FFlecsReplicatedValue& Serialized : Snapshot.Values)
		{
			if (Layout.Keys.IsValidIndex(Serialized.KeyIndex)
				&& Layout.Keys[Serialized.KeyIndex].Kind == EFlecsReplicationKeyKind::Pair
				&& Layout.Keys[Serialized.KeyIndex].TryGetStorageDescriptor(World) == Descriptor)
			{
				Serialized.Bytes.Reset();
				FMemoryWriter Writer(Serialized.Bytes, true);
				FFlecsReplicationTestValueRelationship Copy = Value;
				Descriptor->Serialize(Writer, &Copy);
				return;
			}
		}
	}

	inline void EnqueueRemoval(UFlecsNetworkWorldSubsystem* Subsystem, const FGuid& Shard,
		const FFlecsNetworkId NetworkId)
	{
		FFlecsReplicationInboxRecord Record;
		Record.Type = EFlecsReplicationInboxRecordType::RemoveEntity;
		Record.SourceShard = Shard;
		Record.NetworkId = NetworkId;
		Subsystem->EnqueueReceivedRecord(MoveTemp(Record));
	}
	
}

template<>
struct TFlecsReplicationTraits<UE::Flecs::Tests::FNativeReplicatedValue>
{
	static FString StableName()
	{
		return TEXT("FNativeReplicatedValue");
	}
	
	static bool Serialize(FArchive& Archive, UE::Flecs::Tests::FNativeReplicatedValue& Value)
	{
		Archive << Value.Value;
		return !Archive.IsError();
	}
};

template<>
struct TFlecsReplicationTraits<UE::Flecs::Tests::FNativeReplicatedValueDuplicate>
{
	static FString StableName()
	{
		return TEXT("FNativeReplicatedValue");
	}
	
	static bool Serialize(FArchive& Archive, UE::Flecs::Tests::FNativeReplicatedValueDuplicate& Value)
	{
		Archive << Value.Value;
		return !Archive.IsError();
	}
};

#endif // WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS
