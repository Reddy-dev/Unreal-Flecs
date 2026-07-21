// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CQTest.h"

#if WITH_AUTOMATION_TESTS

template<typename TValue>
struct FFlecsNamedTestCase
{
	FString Name;
	TValue Value;
}; // struct FFlecsNamedTestCase

class FFlecsTestCaseContext
{
public:
	FFlecsTestCaseContext(FAutomationTestBase& InTest, const FString& InName)
		: Test(InTest)
	{
		Test.PushContext(InName);
	}

	~FFlecsTestCaseContext()
	{
		Test.PopContext();
	}

private:
	FAutomationTestBase& Test;
}; // class FFlecsTestCaseContext

template<typename TFixture, typename TValue, typename TFunction>
void ForEachFlecsTestCase(TFixture& InFixture, TConstArrayView<FFlecsNamedTestCase<TValue>> InCases,
	TFunction&& InFunction)
{
	for (const FFlecsNamedTestCase<TValue>& TestCase : InCases)
	{
		FFlecsTestCaseContext Context(*InFixture.TestRunner, TestCase.Name);
		InFunction(TestCase);
	}
}

#endif // WITH_AUTOMATION_TESTS
