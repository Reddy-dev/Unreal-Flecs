// Minimal Unreal spellings required while parsing the Flecs C++ headers.
// This header is never included by generated or runtime code.

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>

#ifndef TEXT
#define TEXT(String) L##String
#endif

struct FBox {};
struct FBoxSphereBounds {};
struct FCapsuleShape {};
struct FIntRect {};
struct FName
{
	FName() = default;
	FName(const wchar_t*) {}
	const wchar_t* ToString() const { return L""; }
};
struct UObject {};
struct UClass
{
	static UClass* StaticClass() { return nullptr; }
};
struct UEnum {};
struct UScriptStruct
{
	static UScriptStruct* StaticClass() { return nullptr; }
	static UScriptStruct* StaticStruct() { return nullptr; }
	const char* GetStructCPPName() const { return ""; }
};
struct UPackage {};

enum class EFindObjectFlags
{
	None,
};

enum EObjectFlags
{
	RF_NoFlags = 0,
};

enum class EInternalObjectFlags
{
	None,
};

inline constexpr int LogClass = 0;
inline constexpr int Fatal = 0;

#ifndef UE_LOG
#define UE_LOG(...) ((void)0)
#endif

template <typename T>
struct TBaseStructure
{
	static UScriptStruct* Get() { return nullptr; }
};

template <typename T>
inline UClass* StaticClass()
{
	return nullptr;
}

template <typename T>
inline UEnum* StaticEnum()
{
	return nullptr;
}

template <typename T, typename... Args>
inline T* FindObjectChecked(Args&&...)
{
	return nullptr;
}

template <typename... Args>
inline UScriptStruct* StaticFindObjectFastInternal(Args&&...)
{
	return nullptr;
}

template <typename T>
struct TWeakObjectPtr
{
	T* Get() const { return nullptr; }
};

template <typename T>
struct TSubclassOf
{
	operator UClass*() const { return nullptr; }
};
