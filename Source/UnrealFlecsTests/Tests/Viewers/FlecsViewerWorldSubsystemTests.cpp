// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Fixtures/FlecsWorldFixture.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "Collections/FlecsCollectionWorldSubsystem.h"
#include "Engine/World.h"
#include "FlecsGameFrameworkModuleSettings.h"
#include "GameFramework/Actor.h"
#include "Viewers/FlecsViewerWorldSubsystem.h"
#include "Viewers/Components/FlecsViewerCollectionTypes.h"
#include "Viewers/Components/FlecsViewerTrackerSingleton.h"
#include "Viewers/Components/FlecsViewerTransformComponent.h"
#include "Viewers/Components/FlecsViewerTypeComponents.h"
#include "Viewers/Systems/FlecsGatherViewersSystems.h"

namespace UE::Flecs::Tests
{
	class FFlecsActorViewerGatherSettingsScope
	{
	public:
		FFlecsActorViewerGatherSettingsScope()
			: Settings(GetMutableDefault<UFlecsGameFrameworkModuleSettings>())
			, bAllowNonPlayerViewerActors(Settings->bAllowNonPlayerViewerActors)
		{
			Settings->bAllowNonPlayerViewerActors = true;
		}

		~FFlecsActorViewerGatherSettingsScope()
		{
			Settings->bAllowNonPlayerViewerActors = bAllowNonPlayerViewerActors;
		}

	private:
		TSolidNotNull<UFlecsGameFrameworkModuleSettings*> Settings;
		bool bAllowNonPlayerViewerActors = false;
	}; // class FFlecsActorViewerGatherSettingsScope

} // namespace UE::Flecs::Tests

FLECS_TEST_CLASS_WITH_FLAGS_AND_TAGS(FlecsViewerWorldSubsystemTests, "UnrealFlecs.Viewers.WorldSubsystem",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
			| EAutomationTestFlags::CriticalPriority, "[Flecs]")
{
protected:
	TSolidNotNull<UFlecsViewerWorldSubsystem*> ViewerSubsystem() const
	{
		return UnrealWorld()->GetSubsystemChecked<UFlecsViewerWorldSubsystem>();
	}

	TSolidNotNull<UFlecsCollectionWorldSubsystem*> CollectionSubsystem() const
	{
		return UnrealWorld()->GetSubsystemChecked<UFlecsCollectionWorldSubsystem>();
	}

public:
	TEST_METHOD(WorldInitialization_InitializesCollectionDependencyBeforeViewerRegistration)
	{
		const TSolidNotNull<UFlecsViewerWorldSubsystem*> Viewers = ViewerSubsystem();
		const TSolidNotNull<UFlecsCollectionWorldSubsystem*> Collections = CollectionSubsystem();

		ASSERT_THAT(IsTrue(Viewers->IsFlecsWorldValid()));
		ASSERT_THAT(IsTrue(Collections->IsFlecsWorldValid()));
		ASSERT_THAT(IsTrue(Viewers->GetFlecsWorld() == World()));
		ASSERT_THAT(IsTrue(Collections->GetFlecsWorld() == World()));

		ASSERT_THAT(IsTrue(Collections->GetPrefabByClass(UFlecsPlayerViewerCollection::StaticClass()).IsValid()));
		ASSERT_THAT(IsTrue(Collections->GetPrefabByClass(UFlecsActorViewerCollection::StaticClass()).IsValid()));
		ASSERT_THAT(IsTrue(Collections->GetPrefabByClass(UFlecsStreamSourceViewerCollection::StaticClass()).IsValid()));
	}

	TEST_METHOD(CreateStreamingSourceViewer_AppliesCollectionParameters)
	{
		const FName SourceName = TEXT("ViewerWorldSubsystemTestSource");
		const FFlecsEntityHandle Viewer = ViewerSubsystem()->AddStreamSourceViewer(SourceName);

		ASSERT_THAT(IsTrue(Viewer.IsValid()));
		ASSERT_THAT(IsTrue(Viewer.HasCollection<UFlecsStreamSourceViewerCollection>()));
		ASSERT_THAT(IsTrue(Viewer.Has<FFlecsViewerTransformComponent>()));

		const FFlecsViewerStreamingSourceComponent* StreamingSource
			= Viewer.TryGetPairSecond<FFlecsViewerRelationship, FFlecsViewerStreamingSourceComponent>();
		ASSERT_THAT(IsTrue(StreamingSource != nullptr));

		if UNLIKELY_IF(StreamingSource == nullptr)
		{
			return;
		}

		ASSERT_THAT(IsTrue(StreamingSource->StreamingSourceName == SourceName));
	}

	TEST_METHOD(GatherActorViewer_TracksAValidActorViewer)
	{
		UE::Flecs::Tests::FFlecsActorViewerGatherSettingsScope SettingsScope;

		AActor* Actor = UnrealWorld()->SpawnActor<AActor>();
		ASSERT_THAT(IsTrue(IsValid(Actor)));

		if UNLIKELY_IF(!IsValid(Actor))
		{
			return;
		}

		const FFlecsEntityHandle Viewer = ViewerSubsystem()->AddActorViewer(Actor);
		ASSERT_THAT(IsTrue(Viewer.IsValid()));

		UFlecsGatherViewersSystem* GatherSystem = World()->RegisterFlecsObject<UFlecsGatherViewersSystem>();
		ASSERT_THAT(IsTrue(IsValid(GatherSystem)));

		if UNLIKELY_IF(!IsValid(GatherSystem))
		{
			return;
		}

		GatherSystem->RunSystem();

		const FFlecsViewerTrackerSingleton& Tracker = World()->Get<FFlecsViewerTrackerSingleton>();
		const FFlecsEntityView* TrackedViewer = Tracker.ActorViewers.Find(Actor);
		ASSERT_THAT(IsTrue(TrackedViewer != nullptr));

		if UNLIKELY_IF(TrackedViewer == nullptr)
		{
			return;
		}

		ASSERT_THAT(IsTrue(TrackedViewer->IsValid()));
		ASSERT_THAT(IsTrue(TrackedViewer->GetRawId() == Viewer.GetRawId()));
	}

}; // FlecsViewerWorldSubsystemTests

#endif // WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS
