// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Fixtures/FlecsWorldFixture.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "UObject/UObjectGlobals.h"
#include "Worlds/FlecsWorld.h"

struct FFlecsTest_CPPStruct_TraitUsesLowId
{
	int32 Value = 0;
}; // struct FFlecsTest_CPPStruct_TraitUsesLowId

template <>
struct TFlecsComponentTraits<FFlecsTest_CPPStruct_TraitUsesLowId>
	: public TFlecsComponentTraitsBase<FFlecsTest_CPPStruct_TraitUsesLowId>
{
	static constexpr bool AutoRegister = true;
	static constexpr bool UseLowId = true;
}; // struct TFlecsComponentTraits<FFlecsTest_CPPStruct_TraitUsesLowId>

struct FFlecsTest_CPPStruct_TraitUsesHighId
{
	int32 Value = 0;
}; // struct FFlecsTest_CPPStruct_TraitUsesHighId

template <>
struct TFlecsComponentTraits<FFlecsTest_CPPStruct_TraitUsesHighId>
	: public TFlecsComponentTraitsBase<FFlecsTest_CPPStruct_TraitUsesHighId>
{
	static constexpr bool AutoRegister = true;
	static constexpr bool UseLowId = false;
}; // struct TFlecsComponentTraits<FFlecsTest_CPPStruct_TraitUsesHighId>

REGISTER_FLECS_COMPONENT(FFlecsTest_CPPStruct_TraitUsesLowId);
REGISTER_FLECS_COMPONENT(FFlecsTest_CPPStruct_TraitUsesHighId);

struct FFlecsTest_CPPStruct_ManualLowId
{
	int32 Value = 0;
}; // struct FFlecsTest_CPPStruct_ManualLowId

template <>
struct TFlecsComponentTraits<FFlecsTest_CPPStruct_ManualLowId>
	: public TFlecsComponentTraitsBase<FFlecsTest_CPPStruct_ManualLowId>
{
	static constexpr bool AutoRegister = false;
}; // struct TFlecsComponentTraits<FFlecsTest_CPPStruct_ManualLowId>

struct FFlecsTest_CPPStruct_ManualHighId
{
	int32 Value = 0;
}; // struct FFlecsTest_CPPStruct_ManualHighId

template <>
struct TFlecsComponentTraits<FFlecsTest_CPPStruct_ManualHighId>
	: public TFlecsComponentTraitsBase<FFlecsTest_CPPStruct_ManualHighId>
{
	static constexpr bool AutoRegister = false;
}; // struct TFlecsComponentTraits<FFlecsTest_CPPStruct_ManualHighId>

REGISTER_FLECS_COMPONENT(FFlecsTest_CPPStruct_ManualLowId);
REGISTER_FLECS_COMPONENT(FFlecsTest_CPPStruct_ManualHighId);

// The normal test fixture registers enough default types to consume the low-ID range.
template <typename TDerived, typename TAsserter>
struct TFlecsLowIdWorldTest : TTest<TDerived, TAsserter>
{
	virtual void Setup() override
	{
		FlecsWorld = NewObject<UFlecsWorld>(GetTransientPackage());
		check(IsValid(FlecsWorld));
		FlecsWorld->AddToRoot();
	}

	virtual void TearDown() override
	{
		if (FlecsWorld)
		{
			FlecsWorld->RemoveFromRoot();
			FlecsWorld->DestroyWorld();
			FlecsWorld = nullptr;
		}
	}

	NO_DISCARD UFlecsWorld* World() const
	{
		return FlecsWorld;
	}

private:
	UFlecsWorld* FlecsWorld = nullptr;
}; // struct TFlecsLowIdWorldTest

namespace
{
	bool IsLowId(const FFlecsEntityHandle& InEntity)
	{
		return InEntity.GetFlecsId().GetIndex() < static_cast<uint32>(FLECS_HI_COMPONENT_ID);
	}
} // namespace

TEST_CLASS_WITH_BASE_AND_FLAGS_AND_TAGS(FlecsComponentLowIdTests,
	"UnrealFlecs.Components.LowId",
	TFlecsLowIdWorldTest,
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
		| EAutomationTestFlags::CriticalPriority,
	"[Flecs][Entity][Component][Registration][LowId]")
{
	TEST_METHOD(ComponentRegistration_Trait_UsesLowId)
	{
		const FFlecsComponentPropertiesDefinition LowIdProperties
			= FFlecsComponentPropertiesDefinition::Make<FFlecsTest_CPPStruct_TraitUsesLowId>();
		ASSERT_THAT(IsTrue(LowIdProperties.bAutoRegister));
		ASSERT_THAT(IsTrue(LowIdProperties.bUseLowId));

		LowIdProperties.RegistrationFunction(World(), LowIdProperties);
		const FFlecsEntityHandle LowIdComponent
			= World()->RegisterComponentType<FFlecsTest_CPPStruct_TraitUsesLowId>();
		ASSERT_THAT(IsTrue(LowIdComponent.IsValid()));
		ASSERT_THAT(IsTrue(LowIdComponent.IsComponent()));
		ASSERT_THAT(IsTrue(IsLowId(LowIdComponent)));

		const FFlecsComponentPropertiesDefinition HighIdProperties
			= FFlecsComponentPropertiesDefinition::Make<FFlecsTest_CPPStruct_TraitUsesHighId>();
		ASSERT_THAT(IsTrue(HighIdProperties.bAutoRegister));
		ASSERT_THAT(IsFalse(HighIdProperties.bUseLowId));

		HighIdProperties.RegistrationFunction(World(), HighIdProperties);
		const FFlecsEntityHandle HighIdComponent
			= World()->RegisterComponentType<FFlecsTest_CPPStruct_TraitUsesHighId>();
		ASSERT_THAT(IsTrue(HighIdComponent.IsValid()));
		ASSERT_THAT(IsTrue(HighIdComponent.IsComponent()));
		ASSERT_THAT(IsFalse(IsLowId(HighIdComponent)));
	}

	TEST_METHOD(ComponentRegistration_Manual_CPPComponent_UsesLowId)
	{
		const FFlecsEntityHandle LowIdComponent
			= World()->RegisterComponentType<FFlecsTest_CPPStruct_ManualLowId>(true);
		const FFlecsEntityHandle HighIdComponent
			= World()->RegisterComponentType<FFlecsTest_CPPStruct_ManualHighId>(false);

		ASSERT_THAT(IsTrue(LowIdComponent.IsValid()));
		ASSERT_THAT(IsTrue(LowIdComponent.IsComponent()));
		ASSERT_THAT(IsTrue(IsLowId(LowIdComponent)));

		ASSERT_THAT(IsTrue(HighIdComponent.IsValid()));
		ASSERT_THAT(IsTrue(HighIdComponent.IsComponent()));
		ASSERT_THAT(IsFalse(IsLowId(HighIdComponent)));
	}

	TEST_METHOD(ComponentRegistration_Manual_ScriptStructType_UsesLowId)
	{
		const FFlecsEntityHandle LowIdComponent
			= World()->RegisterComponentType<FUStructTestComponent_NonTagUSTRUCT>(false, true);
		const FFlecsEntityHandle HighIdComponent
			= World()->RegisterComponentType<FFlecsTestStruct_Value>(false, false);

		ASSERT_THAT(IsTrue(LowIdComponent.IsValid()));
		ASSERT_THAT(IsTrue(LowIdComponent.IsComponent()));
		ASSERT_THAT(IsTrue(IsLowId(LowIdComponent)));

		ASSERT_THAT(IsTrue(HighIdComponent.IsValid()));
		ASSERT_THAT(IsTrue(HighIdComponent.IsComponent()));
		ASSERT_THAT(IsFalse(IsLowId(HighIdComponent)));
	}

	TEST_METHOD(ComponentRegistration_Manual_StaticScriptStructAPI_UsesLowId)
	{
		const FFlecsEntityHandle LowIdComponent
			= World()->RegisterComponentType(FUStructTestComponent_NonTagUSTRUCT::StaticStruct(), false, true);
		const FFlecsEntityHandle HighIdComponent
			= World()->RegisterComponentType(FFlecsTestStruct_Value::StaticStruct(), false, false);

		ASSERT_THAT(IsTrue(LowIdComponent.IsValid()));
		ASSERT_THAT(IsTrue(LowIdComponent.IsComponent()));
		ASSERT_THAT(IsTrue(IsLowId(LowIdComponent)));

		ASSERT_THAT(IsTrue(HighIdComponent.IsValid()));
		ASSERT_THAT(IsTrue(HighIdComponent.IsComponent()));
		ASSERT_THAT(IsFalse(IsLowId(HighIdComponent)));
	}

	TEST_METHOD(ComponentRegistration_Manual_RegisterScriptStructAPI_UsesLowId)
	{
		const FFlecsEntityHandle LowIdComponent
			= World()->RegisterScriptStruct(FUStructTestComponent_NonTagUSTRUCT::StaticStruct(), true, false, true);
		const FFlecsEntityHandle HighIdComponent
			= World()->RegisterScriptStruct(FFlecsTestStruct_Value::StaticStruct(), true, false, false);

		ASSERT_THAT(IsTrue(LowIdComponent.IsValid()));
		ASSERT_THAT(IsTrue(LowIdComponent.IsComponent()));
		ASSERT_THAT(IsTrue(IsLowId(LowIdComponent)));

		ASSERT_THAT(IsTrue(HighIdComponent.IsValid()));
		ASSERT_THAT(IsTrue(HighIdComponent.IsComponent()));
		ASSERT_THAT(IsFalse(IsLowId(HighIdComponent)));
	}

	TEST_METHOD(EnumComponentRegistration_CPPAPI_UsesLowId)
	{
		const FFlecsEntityHandle LowIdComponent
			= World()->RegisterComponentType<ETestEnum>(true);
		const FFlecsEntityHandle HighIdComponent
			= World()->RegisterComponentType<EFlecsTestEnum_SparseUENUM>(false);

		ASSERT_THAT(IsTrue(LowIdComponent.IsValid()));
		ASSERT_THAT(IsTrue(LowIdComponent.IsEnum()));
		ASSERT_THAT(IsTrue(IsLowId(LowIdComponent)));

		ASSERT_THAT(IsTrue(HighIdComponent.IsValid()));
		ASSERT_THAT(IsTrue(HighIdComponent.IsEnum()));
		ASSERT_THAT(IsFalse(IsLowId(HighIdComponent)));
	}

	TEST_METHOD(EnumComponentRegistration_StaticEnumAPI_UsesLowId)
	{
		const FFlecsEntityHandle LowIdComponent
			= World()->RegisterComponentType(StaticEnum<EFlecsTestEnum_UENUM>(), true);
		const FFlecsEntityHandle HighIdComponent
			= World()->RegisterComponentType(StaticEnum<EFlecsTestEnum_SparseUENUM>(), false);

		ASSERT_THAT(IsTrue(LowIdComponent.IsValid()));
		ASSERT_THAT(IsTrue(LowIdComponent.IsEnum()));
		ASSERT_THAT(IsTrue(IsLowId(LowIdComponent)));

		ASSERT_THAT(IsTrue(HighIdComponent.IsValid()));
		ASSERT_THAT(IsTrue(HighIdComponent.IsEnum()));
		ASSERT_THAT(IsFalse(IsLowId(HighIdComponent)));
	}

	TEST_METHOD(EnumRegistration_RegisterScriptEnumAPI_UsesLowId)
	{
		const FFlecsEntityHandle LowIdComponent
			= World()->RegisterScriptEnum(StaticEnum<EFlecsTestEnum_UENUM>(), true);
		const FFlecsEntityHandle HighIdComponent
			= World()->RegisterScriptEnum(StaticEnum<EFlecsTestEnum_SparseUENUM>(), false);

		ASSERT_THAT(IsTrue(LowIdComponent.IsValid()));
		ASSERT_THAT(IsTrue(LowIdComponent.IsEnum()));
		ASSERT_THAT(IsTrue(IsLowId(LowIdComponent)));

		ASSERT_THAT(IsTrue(HighIdComponent.IsValid()));
		ASSERT_THAT(IsTrue(HighIdComponent.IsEnum()));
		ASSERT_THAT(IsFalse(IsLowId(HighIdComponent)));
	}

	TEST_METHOD(EnumRegistration_RegisterScriptEnumTemplateAPI_UsesLowId)
	{
		const FFlecsEntityHandle LowIdComponent = World()->RegisterScriptEnum<ETestEnum>(true);
		const FFlecsEntityHandle HighIdComponent
			= World()->RegisterScriptEnum<EFlecsTestEnum_SparseUENUM>(false);

		ASSERT_THAT(IsTrue(LowIdComponent.IsValid()));
		ASSERT_THAT(IsTrue(LowIdComponent.IsEnum()));
		ASSERT_THAT(IsTrue(IsLowId(LowIdComponent)));

		ASSERT_THAT(IsTrue(HighIdComponent.IsValid()));
		ASSERT_THAT(IsTrue(HighIdComponent.IsEnum()));
		ASSERT_THAT(IsFalse(IsLowId(HighIdComponent)));
	}

	TEST_METHOD(EnumRegistration_RegisterComponentEnumTypeAPI_UsesLowId)
	{
		const FFlecsEntityHandle LowIdComponent
			= World()->RegisterComponentEnumType(StaticEnum<EFlecsTestEnum_UENUM>(), true);
		const FFlecsEntityHandle HighIdComponent
			= World()->RegisterComponentEnumType(StaticEnum<EFlecsTestEnum_SparseUENUM>(), false);

		ASSERT_THAT(IsTrue(LowIdComponent.IsValid()));
		ASSERT_THAT(IsTrue(LowIdComponent.IsEnum()));
		ASSERT_THAT(IsTrue(IsLowId(LowIdComponent)));

		ASSERT_THAT(IsTrue(HighIdComponent.IsValid()));
		ASSERT_THAT(IsTrue(HighIdComponent.IsEnum()));
		ASSERT_THAT(IsFalse(IsLowId(HighIdComponent)));
	}

}; // FlecsComponentLowIdTests

#endif // #if WITH_AUTOMATION_TESTS
