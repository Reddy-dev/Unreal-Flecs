// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "UnrealFlecsConfigMacros.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS && ENABLE_PIE_NETWORK_TEST

#include "Components/PIENetworkComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "Entities/FlecsStablePathTag.h"
#include "GameFramework/GameModeBase.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UObjectIterator.h"

#include "Networking/FlecsNetDirtyTag.h"
#include "Networking/FlecsNetworkId.h"
#include "Networking/FlecsReplicatedEntityComponent.h"
#include "Networking/Profiles/FlecsReplicationProfile.h"
#include "Networking/FlecsReplicationShardSelection.h"
#include "Networking/Layout/FlecsReplicationSnapshot.h"
#include "Networking/Profiles/FlecsReplicationProfileParamTypes.h"
#include "Networking/Shards/FlecsNetEntityProxy.h"
#include "Networking/Shards/FlecsNetEntityTable.h"
#include "Networking/Subsystem/FlecsNetworkWorldSubsystem.h"
#include "Pipelines/FlecsDefaultGameLoop.h"
#include "Queries/FlecsQuery.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"
#include "Worlds/FlecsWorld.h"
#include "Worlds/FlecsWorldSubsystem.h"
#include "Worlds/Settings/FlecsWorldInfoSettings.h"

namespace UE::Flecs::Tests::MissingNetwork
{
	static constexpr TCHAR StablePairTargetName[] = TEXT("StablePairTarget");

	template <typename T>
	static void RegisterReplicationComponent(UFlecsWorld* InWorld)
	{
		const FFlecsComponentHandle Component = InWorld->RegisterComponentType<T>();
		const FFlecsComponentPropertiesDefinition Properties =
			FFlecsComponentPropertiesDefinition::Make<T>();
		Properties.PropertiesFunction(InWorld, Component, Properties);
	}

	static UFlecsWorld* CreateNetworkTestWorld(const UWorld* InWorld, const bool bRegisterSchemaTarget = true)
	{
		const TSolidNotNull<UFlecsWorldSubsystem*> WorldSubsystem =
			InWorld->GetSubsystem<UFlecsWorldSubsystem>();

		FFlecsWorldSettingsInfo Settings;
		Settings.WorldName = TEXT("FlecsReplicationPIEAdditional");
		Settings.GameLoops.AddUnique(NewObject<UFlecsDefaultGameLoop>(WorldSubsystem));

		UFlecsWorld* World = WorldSubsystem->CreateWorld(TEXT("FlecsReplicationPIEAdditional"), Settings);

		RegisterReplicationComponent<FFlecsReplicationTestValueRelationship>(World);
		
		if (bRegisterSchemaTarget)
		{
			RegisterReplicationComponent<FFlecsReplicationTestValue>(World);
		}
		
		RegisterReplicationComponent<FFlecsReplicationTestDontFragmentValue>(World);
		RegisterReplicationComponent<FFlecsReplicationTestNativeValue>(World);
		RegisterReplicationComponent<FFlecsReplicationTestTag>(World);
		RegisterReplicationComponent<FFlecsReplicationTestRequiredTag>(World);
		RegisterReplicationComponent<FFlecsReplicationTestWithValue>(World);
		RegisterReplicationComponent<FFlecsReplicationTestRelationship>(World);

		return World;
	}

	template <typename TState>
	static void EnsureNetworkTestWorld(TState& State, const bool bRegisterSchemaTarget = true)
	{
		if (!State.FlecsWorld)
		{
			State.FlecsWorld = CreateNetworkTestWorld(State.World, bRegisterSchemaTarget);
		}
	}

	static FFlecsEntityHandle FindNetworkEntity(UFlecsWorld* InWorld, const FFlecsNetworkId& InNetworkId)
	{
		if (!InWorld || !InNetworkId.IsValid())
		{
			return FFlecsEntityHandle();
		}

		const UFlecsNetworkWorldSubsystem* NetworkSubsystem =
			InWorld->GetWorld()->GetSubsystemChecked<UFlecsNetworkWorldSubsystem>();
		const TOptional<FFlecsEntityHandle> Entity = NetworkSubsystem->GetEntityFromNetworkId(InNetworkId);
		return Entity.IsSet() ? Entity.GetValue() : FFlecsEntityHandle();
	}

	static FFlecsNetworkId GetNetworkId(const FFlecsEntityHandle& InEntity)
	{
		const FFlecsNetworkId* NetworkId = InEntity.TryGet<FFlecsNetworkId>();
		return NetworkId ? *NetworkId : FFlecsNetworkId();
	}

	static FFlecsEntityHandle FindReplicatedValueEntity(UFlecsWorld* InWorld, const int32 InValue)
	{
		if (!InWorld)
		{
			return FFlecsEntityHandle();
		}

		FFlecsEntityHandle Result;
		const TTypedFlecsQuery<FFlecsReplicationTestValue> Query =
			InWorld->CreateQueryBuilder<const FFlecsReplicationTestValue>()
			.With<FFlecsNetworkId>()
			.Build();

		Query.each([&Result, InValue](flecs::iter& InIter, size_t InIndex,
			const FFlecsReplicationTestValue Value)
		{
			if (!Result.IsValid() && Value.Value == InValue)
			{
				Result = InIter.entity(InIndex);
			}
		});

		return Result;
	}

	static bool HasReplicatedValue(UFlecsWorld* InWorld, const int32 InValue)
	{
		return FindReplicatedValueEntity(InWorld, InValue).IsValid();
	}

	static bool HasValueRelationshipPair(const FFlecsEntityHandle& InEntity, const FFlecsId InTargetId)
	{
		if (!InEntity.IsValid() || !InTargetId.IsValid())
		{
			return false;
		}

		const FFlecsId RelationshipId = InEntity.GetFlecsWorldChecked()
			->RegisterComponentType<FFlecsReplicationTestValueRelationship>().GetFlecsId();
		return InEntity.HasPair(RelationshipId, InTargetId);
	}

	static FFlecsId GetSchemaTargetId(UFlecsWorld* InWorld)
	{
		return InWorld->RegisterComponentType<FFlecsReplicationTestValue>().GetFlecsId();
	}

	static UFlecsNetEntityProxy* FindProxy(const UWorld* InWorld, const FFlecsNetworkId& InNetworkId)
	{
		for (TObjectIterator<UFlecsNetEntityProxy> It; It; ++It)
		{
			UFlecsNetEntityProxy* Proxy = *It;
			if (Proxy->GetWorld() == InWorld && Proxy->NetworkId == InNetworkId)
			{
				return Proxy;
			}
		}

		return nullptr;
	}

	static UFlecsNetEntityTable* FindTable(const UWorld* InWorld, const FFlecsNetworkId& InNetworkId)
	{
		for (TObjectIterator<UFlecsNetEntityTable> It; It; ++It)
		{
			UFlecsNetEntityTable* Table = *It;
			if (Table->GetWorld() != InWorld)
			{
				continue;
			}

			if (Table->EntityTable.Items.ContainsByPredicate(
				[&InNetworkId](const FFlecsNetEntityTableItem& Item)
				{
					return Item.NetworkId == InNetworkId;
				}))
			{
				return Table;
			}
		}

		return nullptr;
	}

	static UFlecsNetEntityTable* FindTableExcept(const UWorld* InWorld,
		const FFlecsNetworkId& InNetworkId, const UFlecsNetEntityTable* InExcludedTable)
	{
		for (TObjectIterator<UFlecsNetEntityTable> It; It; ++It)
		{
			UFlecsNetEntityTable* Table = *It;
			if (Table->GetWorld() != InWorld || Table == InExcludedTable)
			{
				continue;
			}

			if (Table->EntityTable.Items.ContainsByPredicate(
				[&InNetworkId](const FFlecsNetEntityTableItem& Item)
				{
					return Item.NetworkId == InNetworkId;
				}))
			{
				return Table;
			}
		}

		return nullptr;
	}

	static const FFlecsNetEntityTableItem* FindTableItem(const UFlecsNetEntityTable* InTable,
		const FFlecsNetworkId& InNetworkId)
	{
		return InTable
			? InTable->EntityTable.Items.FindByPredicate(
				[&InNetworkId](const FFlecsNetEntityTableItem& Item)
				{
					return Item.NetworkId == InNetworkId;
				})
			: nullptr;
	}

	static bool HasTableEntity(const UWorld* InWorld, const FFlecsNetworkId& InNetworkId)
	{
		return FindTable(InWorld, InNetworkId) != nullptr;
	}
} // namespace UE::Flecs::Tests::MissingNetwork

NETWORK_TEST_CLASS(FlecsReplicationAdditionalRealBridgeNetworkTests,
	"UnrealFlecs.Networking.Replication.RealBridge.PIE.Additional")
{
	struct FState : FBasePIENetworkComponentState
	{
		UFlecsWorld* FlecsWorld = nullptr;
		FFlecsEntityHandle AuthorityEntity;
		FFlecsEntityHandle TargetEntity;
		FFlecsEntityHandle StableTarget;
	};

	FPIENetworkComponent<FState> Network{ TestRunner, TestCommandBuilder, bInitializing };
	FFlecsNetworkId ExpectedNetworkId;
	FFlecsNetworkId ExpectedTargetNetworkId;
	FFlecsNetworkId OldNetworkId;
	FFlecsNetworkId NewNetworkId;
	FFlecsEntityReplicationSnapshot OldSnapshot;
	uint32 ExpectedStateRevision = 0;
	bool bObservedDestinationAndSourceOverlap = false;
	TWeakObjectPtr<UFlecsNetEntityTable> InitialTable;

	BEFORE_EACH()
	{
		ExpectedNetworkId = FFlecsNetworkId();
		ExpectedTargetNetworkId = FFlecsNetworkId();
		OldNetworkId = FFlecsNetworkId();
		NewNetworkId = FFlecsNetworkId();
		OldSnapshot = FFlecsEntityReplicationSnapshot();
		ExpectedStateRevision = 0;
		bObservedDestinationAndSourceOverlap = false;
		InitialTable.Reset();

		FNetworkComponentBuilder<FState>()
			.WithClients(1)
			.AsDedicatedServer()
			.WithGameInstanceClass(UGameInstance::StaticClass())
			.WithGameMode(AGameModeBase::StaticClass())
			.Build(Network);
	}

	TEST_METHOD(PairTargetArrival_SchemaTarget_SourceBeforeTarget)
	{
		Network
			.ThenServer([](FState& State)
			{
				UE::Flecs::Tests::MissingNetwork::EnsureNetworkTestWorld(State);
			})
			.ThenClients([](FState& State)
			{
				UE::Flecs::Tests::MissingNetwork::EnsureNetworkTestWorld(State, false);
			})
			.ThenServer([this](FState& State)
			{
				const FFlecsId SchemaTargetId =
					UE::Flecs::Tests::MissingNetwork::GetSchemaTargetId(State.FlecsWorld);
				
				State.AuthorityEntity = State.FlecsWorld->CreateEntity()
					.SetPair<FFlecsReplicationTestValueRelationship>(
						SchemaTargetId, FFlecsReplicationTestValueRelationship{ 17 })
					.Add<FFlecsReplicatedEntityComponent>();
				
				ExpectedNetworkId = UE::Flecs::Tests::MissingNetwork::GetNetworkId(State.AuthorityEntity);
			})
			.UntilClient(TEXT("Schema pair source arrives before the schema target is registered"), 0,
				[this](FState& State)
			{
				return UE::Flecs::Tests::MissingNetwork::FindNetworkEntity(
					State.FlecsWorld, ExpectedNetworkId).IsValid();
			})
			.ThenClient(0, [](FState& State)
			{
				UE::Flecs::Tests::MissingNetwork::RegisterReplicationComponent<FFlecsReplicationTestValue>(
					State.FlecsWorld);
			})
			.ThenServer([](FState& State)
			{
				State.AuthorityEntity.Add<FFlecsNetDirtyTag>();
			})
			.UntilClient(TEXT("Schema pair is rebuilt after the target schema arrives"), 0,
				[this](FState& State)
			{
				const FFlecsEntityHandle Entity = UE::Flecs::Tests::MissingNetwork::FindNetworkEntity(
					State.FlecsWorld, ExpectedNetworkId);
					
				return UE::Flecs::Tests::MissingNetwork::HasValueRelationshipPair(
					Entity, UE::Flecs::Tests::MissingNetwork::GetSchemaTargetId(State.FlecsWorld));
			});
	}

	TEST_METHOD(PairTargetArrival_SchemaTarget_TargetBeforeSource)
	{
		Network
			.ThenServer([](FState& State)
			{
				UE::Flecs::Tests::MissingNetwork::EnsureNetworkTestWorld(State);
			})
			.ThenClients([](FState& State)
			{
				UE::Flecs::Tests::MissingNetwork::EnsureNetworkTestWorld(State);
			})
			.ThenServer([this](FState& State)
			{
				const FFlecsId SchemaTargetId =
					UE::Flecs::Tests::MissingNetwork::GetSchemaTargetId(State.FlecsWorld);
				State.AuthorityEntity = State.FlecsWorld->CreateEntity()
					.SetPair<FFlecsReplicationTestValueRelationship>(
						SchemaTargetId, FFlecsReplicationTestValueRelationship{ 19 })
					.Add<FFlecsReplicatedEntityComponent>();
				ExpectedNetworkId = UE::Flecs::Tests::MissingNetwork::GetNetworkId(State.AuthorityEntity);
			})
			.UntilClients(TEXT("Schema pair resolves when the schema target is available first"),
				[this](FState& State)
			{
				const FFlecsEntityHandle Entity = UE::Flecs::Tests::MissingNetwork::FindNetworkEntity(
					State.FlecsWorld, ExpectedNetworkId);
				return UE::Flecs::Tests::MissingNetwork::HasValueRelationshipPair(
					Entity, UE::Flecs::Tests::MissingNetwork::GetSchemaTargetId(State.FlecsWorld));
			});
	}

	TEST_METHOD(PairTargetArrival_StableTarget_SourceBeforeTarget)
	{
		Network
			.ThenServer([](FState& State)
			{
				UE::Flecs::Tests::MissingNetwork::EnsureNetworkTestWorld(State);
			})
			.ThenClients([](FState& State)
			{
				UE::Flecs::Tests::MissingNetwork::EnsureNetworkTestWorld(State);
			})
			.ThenServer([this](FState& State)
			{
				const FFlecsEntityHandle Target =
					State.FlecsWorld->CreateEntity(UE::Flecs::Tests::MissingNetwork::StablePairTargetName)
					.Add<FFlecsStablePathTag>();
				
				State.AuthorityEntity = State.FlecsWorld->CreateEntity()
					.SetPair<FFlecsReplicationTestValueRelationship>(
						Target, FFlecsReplicationTestValueRelationship{ 23 })
					.Add<FFlecsReplicatedEntityComponent>();
				ExpectedNetworkId = UE::Flecs::Tests::MissingNetwork::GetNetworkId(State.AuthorityEntity);
			})
			.UntilClient(TEXT("Stable pair source arrives before the stable target"), 0,
				[this](FState& State)
			{
				return UE::Flecs::Tests::MissingNetwork::FindNetworkEntity(
					State.FlecsWorld, ExpectedNetworkId).IsValid();
			})
			.ThenClient(0, [](FState& State)
			{
				State.StableTarget = State.FlecsWorld->CreateEntity(
					UE::Flecs::Tests::MissingNetwork::StablePairTargetName)
					.Add<FFlecsStablePathTag>();
			})
			.ThenServer([](FState& State)
			{
				State.AuthorityEntity.Add<FFlecsNetDirtyTag>();
			})
			.UntilClient(TEXT("Stable pair is rebuilt after the stable target arrives"), 0,
				[this](FState& State)
			{
				const FFlecsEntityHandle Entity = UE::Flecs::Tests::MissingNetwork::FindNetworkEntity(
					State.FlecsWorld, ExpectedNetworkId);
				return UE::Flecs::Tests::MissingNetwork::HasValueRelationshipPair(
					Entity, State.StableTarget.GetFlecsId());
			});
	}

	TEST_METHOD(PairTargetArrival_StableTarget_TargetBeforeSource)
	{
		Network
			.ThenServer([](FState& State)
			{
				UE::Flecs::Tests::MissingNetwork::EnsureNetworkTestWorld(State);
			})
			.ThenClients([](FState& State)
			{
				UE::Flecs::Tests::MissingNetwork::EnsureNetworkTestWorld(State);
				State.StableTarget = State.FlecsWorld->CreateEntity(
					UE::Flecs::Tests::MissingNetwork::StablePairTargetName)
					.Add<FFlecsStablePathTag>();
			})
			.ThenServer([this](FState& State)
			{
				const FFlecsEntityHandle Target =
					State.FlecsWorld->CreateEntity(UE::Flecs::Tests::MissingNetwork::StablePairTargetName)
					.Add<FFlecsStablePathTag>();
				
				State.AuthorityEntity = State.FlecsWorld->CreateEntity()
					.SetPair<FFlecsReplicationTestValueRelationship>(
						Target, FFlecsReplicationTestValueRelationship{ 29 })
					.Add<FFlecsReplicatedEntityComponent>();
				ExpectedNetworkId = UE::Flecs::Tests::MissingNetwork::GetNetworkId(State.AuthorityEntity);
			})
			.UntilClients(TEXT("Stable pair resolves when the stable target is available first"),
				[this](FState& State)
			{
				const FFlecsEntityHandle Entity = UE::Flecs::Tests::MissingNetwork::FindNetworkEntity(
					State.FlecsWorld, ExpectedNetworkId);
				return UE::Flecs::Tests::MissingNetwork::HasValueRelationshipPair(
					Entity, State.StableTarget.GetFlecsId());
			});
	}

	TEST_METHOD(PairTargetArrival_NetworkEntityTarget_SourceBeforeTarget)
	{
		Network
			.ThenServer([](FState& State)
			{
				UE::Flecs::Tests::MissingNetwork::EnsureNetworkTestWorld(State);
			})
			.ThenClients([](FState& State)
			{
				UE::Flecs::Tests::MissingNetwork::EnsureNetworkTestWorld(State);
			})
			.ThenServer([this](FState& State)
			{
				State.TargetEntity = State.FlecsWorld->CreateEntity()
					.Set<FFlecsReplicationTestValue>({ 101 })
					.Add<FFlecsReplicatedEntityComponent>();
				ExpectedTargetNetworkId = UE::Flecs::Tests::MissingNetwork::GetNetworkId(State.TargetEntity);
				State.TargetEntity.Remove<FFlecsNetDirtyTag>();

				State.AuthorityEntity = State.FlecsWorld->CreateEntity()
					.SetPair<FFlecsReplicationTestValueRelationship>(
						State.TargetEntity, FFlecsReplicationTestValueRelationship{ 31 })
					.Add<FFlecsReplicatedEntityComponent>();
				ExpectedNetworkId = UE::Flecs::Tests::MissingNetwork::GetNetworkId(State.AuthorityEntity);
			})
			.UntilClient(TEXT("Network pair source arrives before the network target"), 0,
				[this](FState& State)
			{
				return UE::Flecs::Tests::MissingNetwork::FindNetworkEntity(
					State.FlecsWorld, ExpectedNetworkId).IsValid()
					&& !UE::Flecs::Tests::MissingNetwork::FindNetworkEntity(
						State.FlecsWorld, ExpectedTargetNetworkId).IsValid();
			})
			.ThenServer([](FState& State)
			{
				State.TargetEntity.Add<FFlecsNetDirtyTag>();
			})
			.UntilClient(TEXT("Network target arrives after the source"), 0,
				[this](FState& State)
			{
				return UE::Flecs::Tests::MissingNetwork::FindNetworkEntity(
					State.FlecsWorld, ExpectedTargetNetworkId).IsValid();
			})
			.ThenServer([](FState& State)
			{
				State.AuthorityEntity.Add<FFlecsNetDirtyTag>();
			})
			.UntilClient(TEXT("Network pair resolves after the target arrives"), 0,
				[this](FState& State)
			{
				const FFlecsEntityHandle Source = UE::Flecs::Tests::MissingNetwork::FindNetworkEntity(
					State.FlecsWorld, ExpectedNetworkId);
				const FFlecsEntityHandle Target = UE::Flecs::Tests::MissingNetwork::FindNetworkEntity(
					State.FlecsWorld, ExpectedTargetNetworkId);
				return UE::Flecs::Tests::MissingNetwork::HasValueRelationshipPair(
					Source, Target.GetFlecsId());
			});
	}

	TEST_METHOD(PairTargetArrival_NetworkEntityTarget_TargetBeforeSource)
	{
		Network
			.ThenServer([](FState& State)
			{
				UE::Flecs::Tests::MissingNetwork::EnsureNetworkTestWorld(State);
			})
			.ThenClients([](FState& State)
			{
				UE::Flecs::Tests::MissingNetwork::EnsureNetworkTestWorld(State);
			})
			.ThenServer([this](FState& State)
			{
				State.TargetEntity = State.FlecsWorld->CreateEntity()
					.Set<FFlecsReplicationTestValue>({ 103 })
					.Add<FFlecsReplicatedEntityComponent>();
				ExpectedTargetNetworkId = UE::Flecs::Tests::MissingNetwork::GetNetworkId(State.TargetEntity);
			})
			.UntilClient(TEXT("Network target arrives before the source"), 0,
				[this](FState& State)
			{
				return UE::Flecs::Tests::MissingNetwork::FindNetworkEntity(
					State.FlecsWorld, ExpectedTargetNetworkId).IsValid();
			})
			.ThenServer([this](FState& State)
			{
				State.AuthorityEntity = State.FlecsWorld->CreateEntity()
					.SetPair<FFlecsReplicationTestValueRelationship>(
						State.TargetEntity, FFlecsReplicationTestValueRelationship{ 37 })
					.Add<FFlecsReplicatedEntityComponent>();
				ExpectedNetworkId = UE::Flecs::Tests::MissingNetwork::GetNetworkId(State.AuthorityEntity);
			})
			.UntilClient(TEXT("Network pair resolves when the target arrives first"), 0,
				[this](FState& State)
			{
				const FFlecsEntityHandle Source = UE::Flecs::Tests::MissingNetwork::FindNetworkEntity(
					State.FlecsWorld, ExpectedNetworkId);
				const FFlecsEntityHandle Target = UE::Flecs::Tests::MissingNetwork::FindNetworkEntity(
					State.FlecsWorld, ExpectedTargetNetworkId);
				return UE::Flecs::Tests::MissingNetwork::HasValueRelationshipPair(
					Source, Target.GetFlecsId());
			});
	}

	TEST_METHOD(LateJoin_ReceivesCreatedAndModifiedFlecsEntity)
	{
		Network
			.ThenServer([](FState& State)
			{
				UE::Flecs::Tests::MissingNetwork::EnsureNetworkTestWorld(State);
				State.AuthorityEntity = State.FlecsWorld->CreateEntity()
					.Set<FFlecsReplicationTestValue>({ 43 })
					.Add<FFlecsReplicatedEntityComponent>();
			})
			.ThenClients([](FState& State)
			{
				UE::Flecs::Tests::MissingNetwork::EnsureNetworkTestWorld(State);
			})
			.UntilClient(TEXT("Initial client receives the created entity"), 0,
				[](FState& State)
			{
				return UE::Flecs::Tests::MissingNetwork::HasReplicatedValue(State.FlecsWorld, 43);
			})
			.ThenServer([](FState& State)
			{
				State.AuthorityEntity.Set<FFlecsReplicationTestValue>({ 47 });
			})
			.UntilClient(TEXT("Initial client receives the modification"), 0,
				[](FState& State)
			{
				return UE::Flecs::Tests::MissingNetwork::HasReplicatedValue(State.FlecsWorld, 47);
			})
			.ThenClientJoins()
			.ThenClients([](FState& State)
			{
				UE::Flecs::Tests::MissingNetwork::EnsureNetworkTestWorld(State);
			})
			.UntilClients(TEXT("Late client receives the newest server snapshot"),
				[](FState& State)
			{
				return UE::Flecs::Tests::MissingNetwork::HasReplicatedValue(State.FlecsWorld, 47);
			});
	}

	TEST_METHOD(AuthorityDestroy_DetachesProxyAndCleansClientEntity)
	{
		Network
			.ThenServer([this](FState& State)
			{
				UE::Flecs::Tests::MissingNetwork::EnsureNetworkTestWorld(State);
				State.AuthorityEntity = State.FlecsWorld->CreateEntity()
					.Set<FFlecsReplicationTestValue>({ 53 })
					.Add<FFlecsReplicatedEntityComponent>();
				ExpectedNetworkId = UE::Flecs::Tests::MissingNetwork::GetNetworkId(State.AuthorityEntity);
			})
			.ThenClients([](FState& State)
			{
				UE::Flecs::Tests::MissingNetwork::EnsureNetworkTestWorld(State);
			})
			.UntilClient(0, [](FState& State)
			{
				return UE::Flecs::Tests::MissingNetwork::HasReplicatedValue(State.FlecsWorld, 53);
			})
			.ThenServer([](FState& State)
			{
				State.AuthorityEntity.Destroy();
			})
			.UntilClient(TEXT("Direct authority destruction detaches the client proxy and entity"), 0,
				[this](FState& State)
			{
				const FFlecsEntityHandle Entity = UE::Flecs::Tests::MissingNetwork::FindNetworkEntity(
					State.FlecsWorld, ExpectedNetworkId);
				const UFlecsNetEntityProxy* Proxy =
					UE::Flecs::Tests::MissingNetwork::FindProxy(State.World, ExpectedNetworkId);
				return !Entity.IsValid() && (!Proxy || Proxy->IsEmpty());
			});
	}

	TEST_METHOD(AuthorityDestroy_AfterProxyToTableCleansTableAndClientEntity)
	{
		Network
			.ThenServer([this](FState& State)
			{
				UE::Flecs::Tests::MissingNetwork::EnsureNetworkTestWorld(State);
				State.AuthorityEntity = State.FlecsWorld->CreateEntity()
					.Set<FFlecsReplicationTestValue>({ 55 })
					.Add<FFlecsReplicatedEntityComponent>();
				ExpectedNetworkId = UE::Flecs::Tests::MissingNetwork::GetNetworkId(State.AuthorityEntity);
			})
			.ThenClients([](FState& State)
			{
				UE::Flecs::Tests::MissingNetwork::EnsureNetworkTestWorld(State);
			})
			.UntilClient(0, [](FState& State)
			{
				return UE::Flecs::Tests::MissingNetwork::HasReplicatedValue(State.FlecsWorld, 55);
			})
			.ThenServer([this](FState& State)
			{
				FFlecsReplicationProfileDefinition Profile;
				Profile.ParameterComponents.Add(
					TInstancedStruct<FFlecsReplicationProfileNetShardSelector>::Make(FName(TEXT("Table"))));
				UFlecsNetworkWorldSubsystem* NetworkSubsystem =
					State.World->GetSubsystemChecked<UFlecsNetworkWorldSubsystem>();
				const FFlecsEntityHandle ProfilePrefab = NetworkSubsystem->RegisterReplicationProfileDefinition(
					FName(TEXT("TableDestroyProfile")), Profile);
				ASSERT_THAT(IsTrue(ProfilePrefab.IsValid()));
				ASSERT_THAT(IsTrue(NetworkSubsystem->SetReplicationProfile(State.AuthorityEntity, ProfilePrefab)));
				State.AuthorityEntity.Remove<FFlecsNetDirtyTag>();
				State.AuthorityEntity.Add<FFlecsNetDirtyTag>();
			})
			.UntilClient(TEXT("Entity is hosted by the table before direct destruction"), 0,
				[this](FState& State)
			{
				return UE::Flecs::Tests::MissingNetwork::FindTable(State.World, ExpectedNetworkId)
					&& UE::Flecs::Tests::MissingNetwork::HasReplicatedValue(State.FlecsWorld, 55);
			})
			.ThenServer([](FState& State)
			{
				State.AuthorityEntity.Destroy();
			})
			.UntilClient(TEXT("Direct authority destruction removes the table item and entity"), 0,
				[this](FState& State)
			{
				const FFlecsEntityHandle Entity = UE::Flecs::Tests::MissingNetwork::FindNetworkEntity(
					State.FlecsWorld, ExpectedNetworkId);
				const UFlecsNetEntityTable* Table =
					UE::Flecs::Tests::MissingNetwork::FindTable(State.World, ExpectedNetworkId);
				const UFlecsNetEntityProxy* Proxy =
					UE::Flecs::Tests::MissingNetwork::FindProxy(State.World, ExpectedNetworkId);
				return !Entity.IsValid()
					&& !Table
					&& (!Proxy || Proxy->IsEmpty());
			});
	}

	TEST_METHOD(NetworkIdReuse_StaleSnapshotCannotResurrectRemovedEntity)
	{
		Network
			.ThenServer([this](FState& State)
			{
				UE::Flecs::Tests::MissingNetwork::EnsureNetworkTestWorld(State);
				State.AuthorityEntity = State.FlecsWorld->CreateEntity()
					.Set<FFlecsReplicationTestValue>({ 61 })
					.Add<FFlecsReplicatedEntityComponent>();
				OldNetworkId = UE::Flecs::Tests::MissingNetwork::GetNetworkId(State.AuthorityEntity);
			})
			.ThenClients([](FState& State)
			{
				UE::Flecs::Tests::MissingNetwork::EnsureNetworkTestWorld(State);
			})
			.UntilServer(TEXT("Capture the old network ID and snapshot"), [this](FState& State)
			{
				OldNetworkId = UE::Flecs::Tests::MissingNetwork::GetNetworkId(State.AuthorityEntity);
				const UFlecsNetworkWorldSubsystem* NetworkSubsystem =
					State.World->GetSubsystemChecked<UFlecsNetworkWorldSubsystem>();
				const FFlecsEntityReplicationSnapshot* Snapshot =
					NetworkSubsystem->GetReplicationSnapshots().Find(OldNetworkId);
				if (!Snapshot)
				{
					return false;
				}

				OldSnapshot = *Snapshot;
				return OldNetworkId.IsValid() && OldSnapshot.StateRevision > 0;
			})
			.UntilClient(0, [](FState& State)
			{
				return UE::Flecs::Tests::MissingNetwork::HasReplicatedValue(State.FlecsWorld, 61);
			})
			.ThenServer([](FState& State)
			{
				State.AuthorityEntity.Destroy();
			})
			.UntilClient(TEXT("The old entity is removed before its ID is reused"), 0,
				[this](FState& State)
			{
				return !UE::Flecs::Tests::MissingNetwork::FindNetworkEntity(
					State.FlecsWorld, OldNetworkId).IsValid();
			})
			.ThenServer([this](FState& State)
			{
				State.AuthorityEntity = State.FlecsWorld->CreateEntity()
					.Set<FFlecsReplicationTestValue>({ 67 })
					.Add<FFlecsReplicatedEntityComponent>();
				NewNetworkId = UE::Flecs::Tests::MissingNetwork::GetNetworkId(State.AuthorityEntity);
				ASSERT_THAT(IsTrue(NewNetworkId.IsValid()));
				ASSERT_THAT(IsTrue(NewNetworkId.GetSlot() == OldNetworkId.GetSlot()));
				ASSERT_THAT(IsTrue(NewNetworkId.GetGeneration() != OldNetworkId.GetGeneration()));
			})
			.UntilClient(0, [](FState& State)
			{
				return UE::Flecs::Tests::MissingNetwork::HasReplicatedValue(State.FlecsWorld, 67);
			})
			.ThenClient(0, [this](FState& State)
			{
				UFlecsNetworkWorldSubsystem* NetworkSubsystem =
					State.World->GetSubsystemChecked<UFlecsNetworkWorldSubsystem>();
				NetworkSubsystem->ReceiveNetworkEntitySnapshot(OldNetworkId, OldSnapshot);
				NetworkSubsystem->ApplyQueuedReplicationUpdates(State.FlecsWorld);

				const FFlecsEntityHandle OldEntity = UE::Flecs::Tests::MissingNetwork::FindNetworkEntity(
					State.FlecsWorld, OldNetworkId);
				const FFlecsEntityHandle NewEntity = UE::Flecs::Tests::MissingNetwork::FindNetworkEntity(
					State.FlecsWorld, NewNetworkId);
				ASSERT_THAT(IsFalse(OldEntity.IsValid()));
				ASSERT_THAT(IsTrue(NewEntity.IsValid()));
				ASSERT_THAT(IsTrue(UE::Flecs::Tests::MissingNetwork::HasReplicatedValue(
					State.FlecsWorld, 67)));
			});
	}
	
	TEST_METHOD(ProfileUse_PublishTableEntity)
	{
		Network
			.ThenServer([this](FState& State)
			{
				UE::Flecs::Tests::MissingNetwork::EnsureNetworkTestWorld(State);
				
				FFlecsReplicationProfileDefinition Profile;
				Profile.ParameterComponents.Add(
					TInstancedStruct<FFlecsReplicationProfileNetShardSelector>::Make(FName(TEXT("Table"))));
				UFlecsNetworkWorldSubsystem* NetworkSubsystem =
					State.World->GetSubsystemChecked<UFlecsNetworkWorldSubsystem>();
								
				const FFlecsEntityHandle ProfilePrefab = NetworkSubsystem->RegisterReplicationProfileDefinition(
					FName(TEXT("TableMigrationProfile")), Profile);
				
				State.AuthorityEntity = State.FlecsWorld->CreateEntity()
					.Set<FFlecsReplicationTestValue>({ 69 })
					.Add<FFlecsReplicatedEntityComponent>()
					.AddPrefab(ProfilePrefab);
				
				ExpectedNetworkId = UE::Flecs::Tests::MissingNetwork::GetNetworkId(State.AuthorityEntity);
			})
			.ThenClients([](FState& State)
			{
				UE::Flecs::Tests::MissingNetwork::EnsureNetworkTestWorld(State);
			})
			.UntilClient(TEXT("Table entity is published"), 0,
				[this](FState& State)
				{
					const UFlecsNetEntityTable* Table =
						UE::Flecs::Tests::MissingNetwork::FindTable(State.World, ExpectedNetworkId);
					const FFlecsNetEntityTableItem* Item = UE::Flecs::Tests::MissingNetwork::FindTableItem(Table, ExpectedNetworkId);
					return Item && UE::Flecs::Tests::MissingNetwork::HasReplicatedValue(State.FlecsWorld, 69);
				})
			.ThenServer([this](FState& State)
			{
				State.AuthorityEntity.Set<FFlecsReplicationTestValue>({ 73 });
			})
			.UntilClient(TEXT("Table entity receives updates"), 0,
				[this](FState& State)
				{
					return UE::Flecs::Tests::MissingNetwork::HasReplicatedValue(State.FlecsWorld, 73);
				})
			.ThenServer([this](FState& State)
			{
				State.AuthorityEntity.Destroy();
			})
			.UntilClient(TEXT("Table entity is removed"), 0, [this](FState& State)
				{
					const FFlecsEntityHandle Entity = UE::Flecs::Tests::MissingNetwork::FindNetworkEntity(
						State.FlecsWorld, ExpectedNetworkId);
					const UFlecsNetEntityTable* Table =
						UE::Flecs::Tests::MissingNetwork::FindTable(State.World, ExpectedNetworkId);
					return !Entity.IsValid() && !Table;
				});
			
	}

	TEST_METHOD(ProfileMigration_ProxyToTablePublishesDestinationBaselineBeforeRemoval)
	{
		Network
			.ThenServer([this](FState& State)
			{
				UE::Flecs::Tests::MissingNetwork::EnsureNetworkTestWorld(State);
				State.AuthorityEntity = State.FlecsWorld->CreateEntity()
					.Set<FFlecsReplicationTestValue>({ 71 })
					.Add<FFlecsReplicatedEntityComponent>();
				ExpectedNetworkId = UE::Flecs::Tests::MissingNetwork::GetNetworkId(State.AuthorityEntity);
				
			})
			.ThenClients([](FState& State)
			{
				UE::Flecs::Tests::MissingNetwork::EnsureNetworkTestWorld(State);
			})
			.UntilClient(TEXT("Entity starts on the proxy shard"), 0,
				[this](FState& State)
			{
				const FFlecsEntityHandle Entity = UE::Flecs::Tests::MissingNetwork::FindNetworkEntity(
					State.FlecsWorld, ExpectedNetworkId);
				const UFlecsNetEntityProxy* Proxy =
					UE::Flecs::Tests::MissingNetwork::FindProxy(State.World, ExpectedNetworkId);
					
				return Entity.IsValid() &&
					UE::Flecs::Tests::MissingNetwork::HasReplicatedValue(State.FlecsWorld, 71) &&
					Proxy && !Proxy->IsEmpty();
			})
			.ThenServer([this](FState& State)
			{
				FFlecsReplicationProfileDefinition Profile;
				Profile.ParameterComponents.Add(
					TInstancedStruct<FFlecsReplicationProfileNetShardSelector>::Make(FName(TEXT("Table"))));
				UFlecsNetworkWorldSubsystem* NetworkSubsystem =
					State.World->GetSubsystemChecked<UFlecsNetworkWorldSubsystem>();
				const FFlecsEntityHandle ProfilePrefab = NetworkSubsystem->RegisterReplicationProfileDefinition(
					FName(TEXT("TableMigrationProfile")), Profile);
				
				ASSERT_THAT(IsTrue(ProfilePrefab.IsValid()));
				ASSERT_THAT(IsTrue(
					NetworkSubsystem->SetReplicationProfile(State.AuthorityEntity, ProfilePrefab)));
			})
			.UntilClient(TEXT("Table destination carries a full baseline before the proxy is detached"), 0,
				[this](FState& State)
			{
				const UFlecsNetEntityTable* Table =
					UE::Flecs::Tests::MissingNetwork::FindTable(State.World, ExpectedNetworkId);
				const UFlecsNetEntityProxy* Proxy =
					UE::Flecs::Tests::MissingNetwork::FindProxy(State.World, ExpectedNetworkId);
				if (Table && Proxy && !Proxy->IsEmpty())
				{
					//bObservedDestinationAndSourceOverlap = true;
				}

				const FFlecsNetEntityTableItem* Item =
					UE::Flecs::Tests::MissingNetwork::FindTableItem(Table, ExpectedNetworkId);
				return Item && UE::Flecs::Tests::MissingNetwork::HasReplicatedValue(State.FlecsWorld, 71);
				})
			.ThenClient(0, [this](FState& State)
			{
				const UFlecsNetEntityTable* Table =
					UE::Flecs::Tests::MissingNetwork::FindTable(State.World, ExpectedNetworkId);
				const UFlecsNetEntityProxy* Proxy =
					UE::Flecs::Tests::MissingNetwork::FindProxy(State.World, ExpectedNetworkId);
				//ASSERT_THAT(IsTrue(bObservedDestinationAndSourceOverlap));
				ASSERT_THAT(IsNotNull(Table));
				ASSERT_THAT(IsTrue(UE::Flecs::Tests::MissingNetwork::HasTableEntity(
					State.World, ExpectedNetworkId)));
				ASSERT_THAT(IsTrue(!Proxy || Proxy->IsEmpty()));
			});
	}

	TEST_METHOD(ProfileMigration_TableToTablePublishesDestinationBaselineBeforeRemoval)
	{
		Network
			.ThenServer([this](FState& State)
			{
				UE::Flecs::Tests::MissingNetwork::EnsureNetworkTestWorld(State);

				UFlecsNetworkWorldSubsystem* NetworkSubsystem =
					State.World->GetSubsystemChecked<UFlecsNetworkWorldSubsystem>();
				ASSERT_THAT(IsTrue(NetworkSubsystem->RegisterReplicationShardSelector(
					FName(TEXT("TableAlternate")),
					[](const FFlecsEntityHandle&, const FFlecsNetworkId&, const FFlecsEntityView&,
						OUT FFlecsReplicationShardSelection& OutSelection)
					{
						OutSelection.ShardClass = UFlecsNetEntityTable::StaticClass();
						OutSelection.ShardGroupKey = FName(TEXT("Alternate"));
						return true;
					})));

				FFlecsReplicationProfileDefinition Profile;
				Profile.ParameterComponents.Add(
					TInstancedStruct<FFlecsReplicationProfileNetShardSelector>::Make(FName(TEXT("Table"))));
				const FFlecsEntityHandle ProfilePrefab = NetworkSubsystem->RegisterReplicationProfileDefinition(
					FName(TEXT("TableToTableSourceProfile")), Profile);
				ASSERT_THAT(IsTrue(ProfilePrefab.IsValid()));

				State.AuthorityEntity = State.FlecsWorld->CreateEntity()
					.Set<FFlecsReplicationTestValue>({ 89 })
					.Add<FFlecsReplicatedEntityComponent>()
					.AddPrefab(ProfilePrefab);
				ExpectedNetworkId = UE::Flecs::Tests::MissingNetwork::GetNetworkId(State.AuthorityEntity);
			})
			.ThenClients([](FState& State)
			{
				UE::Flecs::Tests::MissingNetwork::EnsureNetworkTestWorld(State);
			})
			.UntilClient(TEXT("Entity starts on the first table shard"), 0,
				[this](FState& State)
			{
				UFlecsNetEntityTable* Table =
					UE::Flecs::Tests::MissingNetwork::FindTable(State.World, ExpectedNetworkId);
				const FFlecsNetEntityTableItem* Item =
					UE::Flecs::Tests::MissingNetwork::FindTableItem(Table, ExpectedNetworkId);
				if (Item)
				{
					InitialTable = Table;
				}

				return Item && UE::Flecs::Tests::MissingNetwork::HasReplicatedValue(State.FlecsWorld, 89);
			})
			.ThenServer([this](FState& State)
			{
				FFlecsReplicationProfileDefinition Profile;
				Profile.ParameterComponents.Add(
					TInstancedStruct<FFlecsReplicationProfileNetShardSelector>::Make(FName(TEXT("TableAlternate"))));
				UFlecsNetworkWorldSubsystem* NetworkSubsystem =
					State.World->GetSubsystemChecked<UFlecsNetworkWorldSubsystem>();
				const FFlecsEntityHandle ProfilePrefab = NetworkSubsystem->RegisterReplicationProfileDefinition(
					FName(TEXT("TableToTableDestinationProfile")), Profile);

				ASSERT_THAT(IsTrue(ProfilePrefab.IsValid()));
				ASSERT_THAT(IsTrue(
					NetworkSubsystem->SetReplicationProfile(State.AuthorityEntity, ProfilePrefab)));
			})
			.UntilClient(TEXT("The second table carries a full baseline before the first is detached"), 0,
				[this](FState& State)
			{
				UFlecsNetEntityTable* DestinationTable =
					UE::Flecs::Tests::MissingNetwork::FindTableExcept(
						State.World, ExpectedNetworkId, InitialTable.Get());
				const FFlecsNetEntityTableItem* DestinationItem =
					UE::Flecs::Tests::MissingNetwork::FindTableItem(DestinationTable, ExpectedNetworkId);
				if (InitialTable.IsValid() &&
					UE::Flecs::Tests::MissingNetwork::FindTableItem(InitialTable.Get(), ExpectedNetworkId) &&
					DestinationItem)
				{
				//	bObservedDestinationAndSourceOverlap = true;
				}

				return DestinationItem && UE::Flecs::Tests::MissingNetwork::HasReplicatedValue(
					State.FlecsWorld, 89);
			})
			.ThenClient(0, [this](FState& State)
			{
				const UFlecsNetEntityTable* DestinationTable =
					UE::Flecs::Tests::MissingNetwork::FindTableExcept(
						State.World, ExpectedNetworkId, InitialTable.Get());

				//ASSERT_THAT(IsTrue(bObservedDestinationAndSourceOverlap));
				ASSERT_THAT(IsNotNull(DestinationTable));
				ASSERT_THAT(IsTrue(UE::Flecs::Tests::MissingNetwork::FindTableItem(
					DestinationTable, ExpectedNetworkId) != nullptr));
				ASSERT_THAT(IsTrue(!InitialTable.IsValid() ||
					UE::Flecs::Tests::MissingNetwork::FindTableItem(
						InitialTable.Get(), ExpectedNetworkId) == nullptr));
				ASSERT_THAT(IsTrue(UE::Flecs::Tests::MissingNetwork::HasReplicatedValue(
					State.FlecsWorld, 89)));
			});
	}

	TEST_METHOD(ProfileMigration_TableToProxyPublishesDestinationBaselineBeforeRemoval)
	{
		Network
			.ThenServer([this](FState& State)
			{
				UE::Flecs::Tests::MissingNetwork::EnsureNetworkTestWorld(State);

				UFlecsNetworkWorldSubsystem* NetworkSubsystem =
					State.World->GetSubsystemChecked<UFlecsNetworkWorldSubsystem>();
				FFlecsReplicationProfileDefinition Profile;
				Profile.ParameterComponents.Add(
					TInstancedStruct<FFlecsReplicationProfileNetShardSelector>::Make(FName(TEXT("Table"))));
				const FFlecsEntityHandle ProfilePrefab = NetworkSubsystem->RegisterReplicationProfileDefinition(
					FName(TEXT("TableToProxySourceProfile")), Profile);
				ASSERT_THAT(IsTrue(ProfilePrefab.IsValid()));

				State.AuthorityEntity = State.FlecsWorld->CreateEntity()
					.Set<FFlecsReplicationTestValue>({ 97 })
					.Add<FFlecsReplicatedEntityComponent>()
					.AddPrefab(ProfilePrefab);
				ExpectedNetworkId = UE::Flecs::Tests::MissingNetwork::GetNetworkId(State.AuthorityEntity);
			})
			.ThenClients([](FState& State)
			{
				UE::Flecs::Tests::MissingNetwork::EnsureNetworkTestWorld(State);
			})
			.UntilClient(TEXT("Entity starts on the table shard"), 0,
				[this](FState& State)
			{
				UFlecsNetEntityTable* Table =
					UE::Flecs::Tests::MissingNetwork::FindTable(State.World, ExpectedNetworkId);
				const FFlecsNetEntityTableItem* Item =
					UE::Flecs::Tests::MissingNetwork::FindTableItem(Table, ExpectedNetworkId);
				if (Item)
				{
					InitialTable = Table;
				}

				return Item && UE::Flecs::Tests::MissingNetwork::HasReplicatedValue(State.FlecsWorld, 97);
			})
			.ThenServer([this](FState& State)
			{
				FFlecsReplicationProfileDefinition Profile;
				Profile.ParameterComponents.Add(
					TInstancedStruct<FFlecsReplicationProfileNetShardSelector>::Make(FName(TEXT("Proxy"))));
				UFlecsNetworkWorldSubsystem* NetworkSubsystem =
					State.World->GetSubsystemChecked<UFlecsNetworkWorldSubsystem>();
				const FFlecsEntityHandle ProfilePrefab = NetworkSubsystem->RegisterReplicationProfileDefinition(
					FName(TEXT("TableToProxyDestinationProfile")), Profile);

				ASSERT_THAT(IsTrue(ProfilePrefab.IsValid()));
				ASSERT_THAT(IsTrue(
					NetworkSubsystem->SetReplicationProfile(State.AuthorityEntity, ProfilePrefab)));
			})
			.UntilClient(TEXT("The proxy carries a full baseline before the table is detached"), 0,
				[this](FState& State)
			{
				const UFlecsNetEntityProxy* Proxy =
					UE::Flecs::Tests::MissingNetwork::FindProxy(State.World, ExpectedNetworkId);
				if (InitialTable.IsValid() &&
					UE::Flecs::Tests::MissingNetwork::FindTableItem(InitialTable.Get(), ExpectedNetworkId) &&
					Proxy && !Proxy->IsEmpty())
				{
					//bObservedDestinationAndSourceOverlap = true;
				}

				return Proxy && !Proxy->IsEmpty() &&
					UE::Flecs::Tests::MissingNetwork::HasReplicatedValue(State.FlecsWorld, 97);
			})
			.UntilClient(0, [this](FState& State)
			{
				return UE::Flecs::Tests::MissingNetwork::FindTable(State.World, ExpectedNetworkId) == nullptr;
			})
			.ThenClient(0, [this](FState& State)
			{
				const UFlecsNetEntityProxy* Proxy =
					UE::Flecs::Tests::MissingNetwork::FindProxy(State.World, ExpectedNetworkId);
				
				ASSERT_THAT(IsNotNull(Proxy));
				ASSERT_THAT(IsTrue(!Proxy->IsEmpty()));
				ASSERT_THAT(IsTrue(UE::Flecs::Tests::MissingNetwork::FindTable(State.World, ExpectedNetworkId) == nullptr));
				ASSERT_THAT(IsTrue(UE::Flecs::Tests::MissingNetwork::HasReplicatedValue(
					State.FlecsWorld, 97)));
			});
	}
	
}; // FlecsReplicationAdditionalRealBridgeNetworkTests

NETWORK_TEST_CLASS(FlecsReplicationMultipleClientNetworkTests,
	"UnrealFlecs.Networking.Replication.RealBridge.PIE.MultipleClients")
{
	struct FState : FBasePIENetworkComponentState
	{
		UFlecsWorld* FlecsWorld = nullptr;
		FFlecsEntityHandle FirstEntity;
		FFlecsEntityHandle SecondEntity;
	};

	FPIENetworkComponent<FState> Network{ TestRunner, TestCommandBuilder, bInitializing };

	BEFORE_EACH()
	{
		FNetworkComponentBuilder<FState>()
			.WithClients(2)
			.AsDedicatedServer()
			.WithGameInstanceClass(UGameInstance::StaticClass())
			.WithGameMode(AGameModeBase::StaticClass())
			.Build(Network);
	}

	TEST_METHOD(MultipleClients_ReceiveIndependentEntitiesUpdatesAndRemovals)
	{
		Network
			.ThenServer([this](FState& State)
			{
				UE::Flecs::Tests::MissingNetwork::EnsureNetworkTestWorld(State);
				State.FirstEntity = State.FlecsWorld->CreateEntity()
					.Set<FFlecsReplicationTestValue>({ 79 })
					.Add<FFlecsReplicatedEntityComponent>();
				State.SecondEntity = State.FlecsWorld->CreateEntity()
					.Set<FFlecsReplicationTestValue>({ 83 })
					.Add<FFlecsReplicatedEntityComponent>();
			})
			.ThenClients([this](FState& State)
			{
				UE::Flecs::Tests::MissingNetwork::EnsureNetworkTestWorld(State);
			})
			.UntilClients(TEXT("Both clients receive both independent entities"),
				[](FState& State)
			{
				return UE::Flecs::Tests::MissingNetwork::HasReplicatedValue(State.FlecsWorld, 79)
					&& UE::Flecs::Tests::MissingNetwork::HasReplicatedValue(State.FlecsWorld, 83);
			})
			.ThenServer([](FState& State)
			{
				State.FirstEntity.Set<FFlecsReplicationTestValue>({ 89 });
				State.SecondEntity.Remove<FFlecsReplicatedEntityComponent>();
			})
			.UntilClients(TEXT("Both clients receive the update and removal independently"),
				[](FState& State)
			{
				return UE::Flecs::Tests::MissingNetwork::HasReplicatedValue(State.FlecsWorld, 89)
					&& !UE::Flecs::Tests::MissingNetwork::HasReplicatedValue(State.FlecsWorld, 83)
					&& !UE::Flecs::Tests::MissingNetwork::HasReplicatedValue(State.FlecsWorld, 79);
			});
	}
}; // FlecsReplicationMultipleClientNetworkTests

NETWORK_TEST_CLASS(FlecsReplicationListenServerNetworkTests,
	"UnrealFlecs.Networking.Replication.RealBridge.PIE.ListenServer")
{
	struct FState : FBasePIENetworkComponentState
	{
		UFlecsWorld* FlecsWorld = nullptr;
	};

	FPIENetworkComponent<FState> Network{ TestRunner, TestCommandBuilder, bInitializing };

	BEFORE_EACH()
	{
		FNetworkComponentBuilder<FState>()
			.WithClients(1)
			.AsListenServer()
			.WithGameInstanceClass(UGameInstance::StaticClass())
			.WithGameMode(AGameModeBase::StaticClass())
			.Build(Network);
	}

	TEST_METHOD(ListenServer_ReplicatesFlecsEntityToRemoteClient)
	{
		Network
			.ThenServer([this](FState& State)
			{
				ASSERT_THAT(IsTrue(State.World->GetNetMode() == NM_ListenServer));
				UE::Flecs::Tests::MissingNetwork::EnsureNetworkTestWorld(State);
			})
			.ThenClients([this](FState& State)
			{
				ASSERT_THAT(IsTrue(State.World->GetNetMode() == NM_Client));
				UE::Flecs::Tests::MissingNetwork::EnsureNetworkTestWorld(State);
			})
			.ThenServer([](FState& State)
			{
				State.FlecsWorld->CreateEntity()
					.Set<FFlecsReplicationTestValue>({ 97 })
					.Add<FFlecsReplicatedEntityComponent>();
			})
			.UntilClients(TEXT("Listen server replicates the Flecs entity"),
				[](FState& State)
			{
				return UE::Flecs::Tests::MissingNetwork::HasReplicatedValue(State.FlecsWorld, 97);
			});
	}
}; // FlecsReplicationListenServerNetworkTests

NETWORK_TEST_CLASS(FlecsReplicationPacketSimulationNetworkTests,
	"UnrealFlecs.Networking.Replication.RealBridge.PIE.PacketSimulation")
{
	struct FState : FBasePIENetworkComponentState
	{
		UFlecsWorld* FlecsWorld = nullptr;
		FFlecsEntityHandle AuthorityEntity;
	};

	FPIENetworkComponent<FState> Network{ TestRunner, TestCommandBuilder, bInitializing };
	FPacketSimulationSettings PacketSimulationSettings;
	FFlecsNetworkId ExpectedNetworkId;
	uint32 ExpectedStateRevision = 0;

	BEFORE_EACH()
	{
		PacketSimulationSettings.ResetSettings();
		PacketSimulationSettings.PktLoss = 20;
		PacketSimulationSettings.PktOrder = 1;
		PacketSimulationSettings.PktJitter = 5;
		PacketSimulationSettings.PktDup = 3;
		ExpectedNetworkId = FFlecsNetworkId();
		ExpectedStateRevision = 0;

		FNetworkComponentBuilder<FState>()
			.WithClients(1)
			.AsDedicatedServer()
			.WithPacketSimulationSettings(&PacketSimulationSettings)
			.WithGameInstanceClass(UGameInstance::StaticClass())
			.WithGameMode(AGameModeBase::StaticClass())
			.Build(Network);
	}

	TEST_METHOD(PacketSimulation_ReorderedAndLostSnapshotsEndAtNewestStateRevision)
	{
		Network
			.ThenServer([this](FState& State)
			{
				UE::Flecs::Tests::MissingNetwork::EnsureNetworkTestWorld(State);
				State.AuthorityEntity = State.FlecsWorld->CreateEntity()
					.Set<FFlecsReplicationTestValue>({ 107 })
					.Add<FFlecsReplicatedEntityComponent>();
				ExpectedNetworkId = UE::Flecs::Tests::MissingNetwork::GetNetworkId(State.AuthorityEntity);
			})
			.ThenClients([](FState& State)
			{
				UE::Flecs::Tests::MissingNetwork::EnsureNetworkTestWorld(State);
			})
			.UntilClient(0, [](FState& State)
			{
				return UE::Flecs::Tests::MissingNetwork::HasReplicatedValue(State.FlecsWorld, 107);
			})
			.ThenServer([](FState& State)
			{
				State.AuthorityEntity.Set<FFlecsReplicationTestValue>({ 109 });
			})
			.ThenServer([](FState& State)
			{
				State.AuthorityEntity.Set<FFlecsReplicationTestValue>({ 113 });
			})
			.ThenServer([](FState& State)
			{
				State.AuthorityEntity.Set<FFlecsReplicationTestValue>({ 127 });
			})
			.UntilServer(TEXT("Capture the newest authoritative state revision"), [this](FState& State)
			{
				const UFlecsNetworkWorldSubsystem* NetworkSubsystem =
					State.World->GetSubsystemChecked<UFlecsNetworkWorldSubsystem>();
				const FFlecsEntityReplicationSnapshot* Snapshot =
					NetworkSubsystem->GetReplicationSnapshots().Find(ExpectedNetworkId);
				if (!Snapshot)
				{
					return false;
				}

				ExpectedStateRevision = Snapshot->StateRevision;
				return ExpectedStateRevision > 0
					&& UE::Flecs::Tests::MissingNetwork::HasReplicatedValue(State.FlecsWorld, 127);
			})
			.UntilClient(TEXT("Client ends on the newest snapshot revision"), 0,
				[this](FState& State)
			{
				const UFlecsNetworkWorldSubsystem* NetworkSubsystem =
					State.World->GetSubsystemChecked<UFlecsNetworkWorldSubsystem>();
				const FFlecsEntityReplicationSnapshot* Snapshot =
					NetworkSubsystem->GetReplicationSnapshots().Find(ExpectedNetworkId);
				return Snapshot
					&& Snapshot->StateRevision >= ExpectedStateRevision
					&& UE::Flecs::Tests::MissingNetwork::HasReplicatedValue(State.FlecsWorld, 127);
			});
	}
}; // FlecsReplicationPacketSimulationNetworkTests

#endif // WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS && ENABLE_PIE_NETWORK_TEST
