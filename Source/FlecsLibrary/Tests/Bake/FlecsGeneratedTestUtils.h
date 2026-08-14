// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if WITH_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include <string>

#ifdef test_assert
#undef test_assert
#endif
#ifdef test_bool
#undef test_bool
#endif
#ifdef test_true
#undef test_true
#endif
#ifdef test_false
#undef test_false
#endif
#ifdef test_int
#undef test_int
#endif
#ifdef test_uint
#undef test_uint
#endif
#ifdef test_flt
#undef test_flt
#endif
#ifdef test_str
#undef test_str
#endif
#ifdef test_null
#undef test_null
#endif
#ifdef test_not_null
#undef test_not_null
#endif
#ifdef test_ptr
#undef test_ptr
#endif

namespace FlecsGeneratedTest
{
	inline thread_local bool bFailed = false;
	inline thread_local bool bSkipped = false;
	inline thread_local FString Source;
	inline thread_local FString Case;

	inline FString MakeMessage(const TCHAR* Description)
	{
		return FString::Printf(TEXT("[%s:%s] %s"), *Source, *Case, Description);
	}

	inline void Begin(const TCHAR* InSource, const TCHAR* InCase)
	{
		bFailed = false;
		bSkipped = false;
		Source = InSource;
		Case = InCase;
	}

	inline bool Record(const bool bPassed, const TCHAR* Description)
	{
		bFailed |= !bPassed;
		if (FAutomationTestBase* CurrentTest = FAutomationTestFramework::Get().GetCurrentTest())
		{
			CurrentTest->TestTrue(MakeMessage(Description), bPassed);
		}
		return bPassed;
	}

	template <typename TActual, typename TExpected>
	inline bool Equal(const TActual& Actual, const TExpected& Expected, const TCHAR* Description)
	{
		return Record(Actual == Expected, Description);
	}

	template <typename TActual, typename TExpected>
	inline bool PointerEqual(const TActual& Actual, const TExpected& Expected, const TCHAR* Description)
	{
		return Record(Actual == Expected, Description);
	}

	inline void Unsupported(const TCHAR* Reason, const int32 Line)
	{
		const FString Description = FString::Printf(TEXT("Unsupported conversion at source line %d: %s"), Line, Reason);
		Record(false, *Description);
	}

	inline void Skip(const TCHAR* Reason)
	{
		bSkipped = true;
		if (FAutomationTestBase* CurrentTest = FAutomationTestFramework::Get().GetCurrentTest())
		{
			CurrentTest->TestTrue(MakeMessage(Reason), true);
		}
	}

	inline bool End()
	{
		return !bFailed;
	}
}

#define test_assert(Condition) \
	do \
	{ \
		const bool bFlecsGeneratedResult = !!(Condition); \
		FlecsGeneratedTest::Record(bFlecsGeneratedResult, TEXT(#Condition)); \
	} while (false)

#define test_bool(Value1, Value2) \
	do \
	{ \
		FlecsGeneratedTest::Equal((Value1), (Value2), TEXT(#Value1 " == " #Value2)); \
	} while (false)

#define test_true(Value) test_bool((Value), true)
#define test_false(Value) test_bool((Value), false)
#define test_int(Value1, Value2) test_bool(static_cast<int64>(Value1), static_cast<int64>(Value2))
#define test_uint(Value1, Value2) test_bool(static_cast<uint64>(Value1), static_cast<uint64>(Value2))
#define test_flt(Value1, Value2) test_bool(static_cast<float>(Value1), static_cast<float>(Value2))
#define test_str(Value1, Value2) test_bool(std::string(Value1), std::string(Value2))
#define test_null(Value) test_assert((Value) == nullptr)
#define test_not_null(Value) test_assert((Value) != nullptr)
#define test_ptr(Value1, Value2) \
	do \
	{ \
		FlecsGeneratedTest::PointerEqual((Value1), (Value2), TEXT(#Value1 " == " #Value2)); \
	} while (false)

#endif // WITH_AUTOMATION_TESTS
