// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "CQTest.h"
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "General/FlecsEntitySettings.h"
#include "Observers/FlecsObserverDefinition.h"
#include "Observers/FlecsObserverEventInput.h"
#include "Observers/FlecsObserverFlags.h"
#include "Systems/FlecsSystemDefinition.h"
#include "Systems/FlecsSystemObject.h"
#include "Systems/FlecsSystemPhaseInput.h"
#include "Systems/FlecsSystemPipelineInput.h"
#include "Systems/FlecsSystemTickSourceInput.h"
#include "Templates/UnrealTemplate.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "UnrealFlecsTests/Tests/Types/FlecsDefinitionOverrideTestTypes.h"

TEST_CLASS_WITH_FLAGS_AND_TAGS(FlecsDefinitionOverrideTests,
	"UnrealFlecs.Settings.DefinitionOverrides",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
	| EAutomationTestFlags::CriticalPriority,
	"[Flecs][Settings][Overrides]")
{
	TEST_METHOD(SystemDefinitionOverrides_ReplaceConfiguredFields)
	{
		UFlecsSystemDefinitionOverrideTestObject* TestObject = NewObject<UFlecsSystemDefinitionOverrideTestObject>();

		FFlecsSystemDefinitionOverrides Overrides;

		FFlecsSystemPhaseInput PhaseOverride;
		PhaseOverride.Type = EFlecsSystemPhaseInputType::FlecsPhase;
		PhaseOverride.FlecsPhase = EFlecsPhaseType::PostUpdate;
		Overrides.PhaseInputOverride.Emplace(PhaseOverride);
		Overrides.IntervalOverride.Emplace(2.5);
		Overrides.RateOverride.Emplace(7u);

		FFlecsSystemTickSourceInput TickSourceOverride;
		TickSourceOverride.InputType = EFlecsSystemTickSourceInput::SystemClass;
		TickSourceOverride.SystemClassInput = UFlecsSystemDefinitionOverrideTestObject::StaticClass();
		Overrides.TickSourceInputOverride.Emplace(TickSourceOverride);
		Overrides.MultiThreadedOverride.Emplace(true);
		Overrides.ImmediateOverride.Emplace(true);

		FFlecsSystemPipelineInput PipelineOverride;
		PipelineOverride.InputType = EFlecsSystemPipelineInputType::Type;
		Overrides.PipelineInputOverride.Emplace(PipelineOverride);

		TestObject->SetDefinitionOverrides(Overrides);

		FFlecsSystemDefinition Definition;
		Definition.PhaseInput.Type = EFlecsSystemPhaseInputType::FlecsPhase;
		Definition.PhaseInput.FlecsPhase = EFlecsPhaseType::PreUpdate;
		Definition.Interval = 0.25;
		Definition.Rate = 1u;
		Definition.TickSourceInput.InputType = EFlecsSystemTickSourceInput::None;
		Definition.bMultiThreaded = false;
		Definition.bImmediate = false;
		Definition.PipelineInput.InputType = EFlecsSystemPipelineInputType::None;

		TestObject->ApplyDefinitionOverrides(Definition);

		ASSERT_THAT(AreEqual(Definition.PhaseInput.Type, EFlecsSystemPhaseInputType::FlecsPhase));
		ASSERT_THAT(AreEqual(Definition.PhaseInput.FlecsPhase, EFlecsPhaseType::PostUpdate));
		ASSERT_THAT(IsTrue(Definition.Interval == 2.5));
		ASSERT_THAT(AreEqual(Definition.Rate, 7u));
		ASSERT_THAT(AreEqual(Definition.TickSourceInput.InputType, EFlecsSystemTickSourceInput::SystemClass));
		ASSERT_THAT(IsTrue(Definition.TickSourceInput.SystemClassInput.Get()
			== UFlecsSystemDefinitionOverrideTestObject::StaticClass()));
		ASSERT_THAT(IsTrue(Definition.bMultiThreaded));
		ASSERT_THAT(IsTrue(Definition.bImmediate));
		ASSERT_THAT(AreEqual(Definition.PipelineInput.InputType, EFlecsSystemPipelineInputType::Type));
	}

	TEST_METHOD(SystemDefinitionOverrides_UnsetFieldsPreserveDefinition)
	{
		UFlecsSystemDefinitionOverrideTestObject* TestObject = NewObject<UFlecsSystemDefinitionOverrideTestObject>();
		TestObject->SetDefinitionOverrides(FFlecsSystemDefinitionOverrides());

		FFlecsSystemDefinition Definition;
		Definition.PhaseInput.Type = EFlecsSystemPhaseInputType::FlecsPhase;
		Definition.PhaseInput.FlecsPhase = EFlecsPhaseType::OnUpdate;
		Definition.Interval = 4.0;
		Definition.Rate = 3u;
		Definition.TickSourceInput.InputType = EFlecsSystemTickSourceInput::SystemClass;
		Definition.TickSourceInput.SystemClassInput = UFlecsSystemDefinitionOverrideTestObject::StaticClass();
		Definition.bMultiThreaded = true;
		Definition.bImmediate = true;
		Definition.PipelineInput.InputType = EFlecsSystemPipelineInputType::Type;

		TestObject->ApplyDefinitionOverrides(Definition);

		ASSERT_THAT(AreEqual(Definition.PhaseInput.FlecsPhase, EFlecsPhaseType::OnUpdate));
		ASSERT_THAT(IsTrue(Definition.Interval == 4.0));
		ASSERT_THAT(AreEqual(Definition.Rate, 3u));
		ASSERT_THAT(AreEqual(Definition.TickSourceInput.InputType, EFlecsSystemTickSourceInput::SystemClass));
		ASSERT_THAT(IsTrue(Definition.bMultiThreaded));
		ASSERT_THAT(IsTrue(Definition.bImmediate));
		ASSERT_THAT(AreEqual(Definition.PipelineInput.InputType, EFlecsSystemPipelineInputType::Type));
	}

	TEST_METHOD(SystemDefinitionOverrides_CDOValuesAreCopiedToInstances)
	{
		UFlecsSystemDefinitionOverrideTestObject* CDO =
			GetMutableDefault<UFlecsSystemDefinitionOverrideTestObject>();
		const FFlecsSystemDefinitionOverrides PreviousOverrides = CDO->GetDefinitionOverrides();
		TGuardValue<FFlecsSystemDefinitionOverrides> RestoreOverrides(CDO->GetDefinitionOverrides(), PreviousOverrides);

		FFlecsSystemDefinitionOverrides CDOOverrides;
		CDOOverrides.IntervalOverride.Emplace(8.0);
		CDO->SetDefinitionOverrides(CDOOverrides);

		UFlecsSystemDefinitionOverrideTestObject* Instance = NewObject<UFlecsSystemDefinitionOverrideTestObject>();
		ASSERT_THAT(IsNotNull(Instance));
		ASSERT_THAT(IsTrue(Instance->GetDefinitionOverrides().IntervalOverride.IsSet()));
		ASSERT_THAT(IsTrue(Instance->GetDefinitionOverrides().IntervalOverride.GetValue() == 8.0));
	}

	TEST_METHOD(ObserverDefinitionOverrides_AppendEventsAndFlags)
	{
		UFlecsObserverDefinitionOverrideTestObject* TestObject = NewObject<UFlecsObserverDefinitionOverrideTestObject>();

		FFlecsObserverDefinitionOverrides Overrides;
		Overrides.EventsOverride.Add(FFlecsObserverEventInput::Make(EFlecsObserverEvent::OnSet));
		Overrides.YieldExistingOverride.Emplace(true);
		Overrides.FlagsOverride = static_cast<uint32>(EFlecsObserverFlags::MatchDisabled);
		TestObject->SetDefinitionOverrides(Overrides);

		FFlecsObserverDefinition Definition;
		Definition.Events.Add(FFlecsObserverEventInput::Make(EFlecsObserverEvent::OnAdd));
		Definition.Events.Add(FFlecsObserverEventInput::Make(EFlecsObserverEvent::OnRemove));
		Definition.bYieldExisting = false;
		Definition.Flags = static_cast<uint32>(EFlecsObserverFlags::MatchPrefab);

		TestObject->ApplyDefinitionOverrides(Definition);

		ASSERT_THAT(AreEqual(Definition.Events.Num(), 3));
		ASSERT_THAT(AreEqual(Definition.Events[0].EventType, EFlecsObserverEvent::OnAdd));
		ASSERT_THAT(AreEqual(Definition.Events[1].EventType, EFlecsObserverEvent::OnRemove));
		ASSERT_THAT(AreEqual(Definition.Events[2].EventType, EFlecsObserverEvent::OnSet));
		ASSERT_THAT(IsTrue(Definition.bYieldExisting));
		ASSERT_THAT(AreEqual(Definition.Flags,
			static_cast<uint32>(EFlecsObserverFlags::MatchPrefab | EFlecsObserverFlags::MatchDisabled)));
	}

	TEST_METHOD(ObserverDefinitionOverrides_ReplaceEventsAndFlags)
	{
		UFlecsObserverDefinitionOverrideTestObject* TestObject = NewObject<UFlecsObserverDefinitionOverrideTestObject>();

		FFlecsObserverDefinitionOverrides Overrides;
		Overrides.bOverrideObserverEvents = true;
		Overrides.EventsOverride.Add(FFlecsObserverEventInput::Make(EFlecsObserverEvent::OnDelete));
		Overrides.YieldExistingOverride.Emplace(false);
		Overrides.bOverrideObserverFlags = true;
		Overrides.FlagsOverride = static_cast<uint32>(EFlecsObserverFlags::IsMonitor);
		TestObject->SetDefinitionOverrides(Overrides);

		FFlecsObserverDefinition Definition;
		Definition.Events.Add(FFlecsObserverEventInput::Make(EFlecsObserverEvent::OnAdd));
		Definition.Events.Add(FFlecsObserverEventInput::Make(EFlecsObserverEvent::OnRemove));
		Definition.bYieldExisting = true;
		Definition.Flags = static_cast<uint32>(EFlecsObserverFlags::MatchPrefab);

		TestObject->ApplyDefinitionOverrides(Definition);

		ASSERT_THAT(AreEqual(Definition.Events.Num(), 1));
		ASSERT_THAT(AreEqual(Definition.Events[0].EventType, EFlecsObserverEvent::OnDelete));
		ASSERT_THAT(IsFalse(Definition.bYieldExisting));
		ASSERT_THAT(AreEqual(Definition.Flags, static_cast<uint32>(EFlecsObserverFlags::IsMonitor)));
	}

	TEST_METHOD(ObserverDefinitionOverrides_CDOValuesAreCopiedToInstances)
	{
		UFlecsObserverDefinitionOverrideTestObject* CDO =
			GetMutableDefault<UFlecsObserverDefinitionOverrideTestObject>();
		const FFlecsObserverDefinitionOverrides PreviousOverrides = CDO->GetDefinitionOverrides();
		TGuardValue RestoreOverrides(CDO->GetDefinitionOverrides(), PreviousOverrides);

		FFlecsObserverDefinitionOverrides CDOOverrides;
		CDOOverrides.bOverrideObserverEvents = true;
		CDOOverrides.EventsOverride.Add(FFlecsObserverEventInput::Make(EFlecsObserverEvent::OnDelete));
		CDO->SetDefinitionOverrides(CDOOverrides);

		UFlecsObserverDefinitionOverrideTestObject* Instance = NewObject<UFlecsObserverDefinitionOverrideTestObject>();
		ASSERT_THAT(IsNotNull(Instance));
		ASSERT_THAT(IsTrue(Instance->GetDefinitionOverrides().bOverrideObserverEvents));
		ASSERT_THAT(AreEqual(Instance->GetDefinitionOverrides().EventsOverride.Num(), 1));
		ASSERT_THAT(AreEqual(Instance->GetDefinitionOverrides().EventsOverride[0].EventType,
			EFlecsObserverEvent::OnDelete));
	}

	/*TEST_METHOD(EntitySettings_ListsOverrideObjectCDOsAndConfigProperties)
	{
		UFlecsEntitySettings* EntitySettings = GetMutableDefault<UFlecsEntitySettings>();
		EntitySettings->BuildSystemList();

		const UObject* SystemCDO = GetDefault<UFlecsSystemDefinitionOverrideTestObject>();
		const UObject* ObserverCDO = GetDefault<UFlecsObserverDefinitionOverrideTestObject>();

		const bool bHasSystemCDO = EntitySettings->CDOs.ContainsByPredicate(
			[SystemCDO](const TObjectPtr<const UObject>& InCDO)
			{
				return InCDO.Get() == SystemCDO;
			});
		
		const bool bHasObserverCDO = EntitySettings->CDOs.ContainsByPredicate(
			[ObserverCDO](const TObjectPtr<const UObject>& InCDO)
			{
				return InCDO.Get() == ObserverCDO;
			});

		ASSERT_THAT(IsTrue(bHasSystemCDO));
		ASSERT_THAT(IsTrue(bHasObserverCDO));

		const FProperty* SystemOverridesProperty =
			UFlecsSystemDefinitionOverrideTestObject::StaticClass()->FindPropertyByName(TEXT("SystemDefinitionOverrides"));
		const FProperty* ObserverOverridesProperty =
			UFlecsObserverDefinitionOverrideTestObject::StaticClass()->FindPropertyByName(TEXT("ObserverDefinitionOverrides"));

		ASSERT_THAT(IsNotNull(SystemOverridesProperty));
		ASSERT_THAT(IsNotNull(ObserverOverridesProperty));
		ASSERT_THAT(IsTrue(SystemOverridesProperty->HasAllPropertyFlags(CPF_Edit | CPF_Config)));
		ASSERT_THAT(IsTrue(ObserverOverridesProperty->HasAllPropertyFlags(CPF_Edit | CPF_Config)));

		const FStructProperty* SystemOverridesStructProperty = CastField<const FStructProperty>(SystemOverridesProperty);
		const FStructProperty* ObserverOverridesStructProperty = CastField<const FStructProperty>(ObserverOverridesProperty);
		ASSERT_THAT(IsNotNull(SystemOverridesStructProperty));
		ASSERT_THAT(IsNotNull(ObserverOverridesStructProperty));

		const UScriptStruct* SystemOverridesStruct = SystemOverridesStructProperty->Struct;
		const UScriptStruct* ObserverOverridesStruct = ObserverOverridesStructProperty->Struct;
		ASSERT_THAT(IsNotNull(SystemOverridesStruct));
		ASSERT_THAT(IsNotNull(ObserverOverridesStruct));
	}*/
	
}; // FlecsDefinitionOverrideTests*/

#endif // WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS
