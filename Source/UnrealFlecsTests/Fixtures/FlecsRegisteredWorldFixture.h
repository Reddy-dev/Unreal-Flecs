// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "UnrealFlecsTests/Fixtures/FlecsWorldFixture.h"
#include "UnrealFlecsTests/Tests/FlecsTestTypes.h"

#if WITH_AUTOMATION_TESTS

template<typename TDerived, typename TAsserter>
struct TFlecsRegisteredWorldTest : TFlecsWorldTest<TDerived, TAsserter>
{
	using Super = TFlecsWorldTest<TDerived, TAsserter>;

protected:
	virtual void OnWorldSetUp() override
	{
		Super::OnWorldSetUp();

		this->World()->RegisterComponentType<FFlecsTest_CPPStruct>();
		this->World()->RegisterComponentType<FFlecsTest_CPPStructValue>();
		this->World()->RegisterComponentType<FFlecsTestStruct_Tag>();
		this->World()->RegisterComponentType<FFlecsTestStruct_Value>();
		this->World()->RegisterComponentType<FUSTRUCTPairTestComponent>();
		this->World()->RegisterComponentType<FUSTRUCTPairTestComponent_Second>();
		this->World()->RegisterComponentType<FUSTRUCTPairTestComponent_Data>();

		OnRegisteredWorldSetUp();
	}

	virtual void OnRegisteredWorldSetUp()
	{
	}
}; // struct TFlecsRegisteredWorldTest

#define FLECS_REGISTERED_TEST_CLASS_WITH_FLAGS(_ClassName, _TestDir, _Flags) \
	TEST_CLASS_WITH_BASE_AND_FLAGS(_ClassName, _TestDir, TFlecsRegisteredWorldTest, _Flags)

#define FLECS_REGISTERED_TEST_CLASS_WITH_FLAGS_AND_TAGS(_ClassName, _TestDir, _Flags, _TestTags) \
	TEST_CLASS_WITH_BASE_AND_FLAGS_AND_TAGS(_ClassName, _TestDir, TFlecsRegisteredWorldTest, _Flags, _TestTags)

#endif // WITH_AUTOMATION_TESTS
