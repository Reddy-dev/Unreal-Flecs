// Solstice Games © 2024. All Rights Reserved.

#ifndef SOLID_MACROS_STANDARD_HASHING_H
#define SOLID_MACROS_STANDARD_HASHING_H

#include <vector>
#include <functional>
#include <type_traits>


#include "Templates/TypeHash.h"
#include "UObject/ObjectKey.h"
#include "GameplayTagsManager.h"

#include "SolidMacros/Macros.h"

namespace Solid
{
	/**
	 * Combines two or more already-computed hash values using Unreal's stable
	 * HashCombine implementation.
	 */
	template <typename... THashTypes>
	requires (sizeof...(THashTypes) >= 2 && (std::is_convertible_v<THashTypes, uint32> && ...))
	NO_DISCARD SOLID_INLINE constexpr uint32 HashCombine(THashTypes... InHashes)
	{
		return ::HashCombine(static_cast<uint32>(InHashes)...);
	}
	
	/**
	 * Combines two or more already-computed hash values using Unreal's fast,
	 * process-local HashCombineFast implementation.
	 */
	template <typename... THashTypes>
	requires (sizeof...(THashTypes) >= 2 && (std::is_convertible_v<THashTypes, uint32> && ...))
	NO_DISCARD SOLID_INLINE constexpr uint32 HashCombineFast(THashTypes... InHashes)
	{
		return ::HashCombineFast(static_cast<uint32>(InHashes)...);
	}

} // namespace Solid

#define DEFINE_STD_HASH(x) \
	template <> \
	struct std::hash<x> \
	{ \
	public: \
		SOLID_INLINE std::size_t operator()(const x& Value) const noexcept \
		{ \
			return GetTypeHash(Value); \
		} \
		\
	}; // struct std::hash<x>

#define DEFINE_STD_HASH_CUSTOM_FUNC(x, FUNC) \
	template <> \
	struct std::hash<x> \
	{ \
	public: \
		SOLID_INLINE std::size_t operator()(const x& Value) const noexcept \
		{ \
			return FUNC(Value); \
		} \
		\
	}; // struct std::hash<x>

#define DEFINE_STD_HASH_TEMPLATED(x, ...) \
	template <##__VA_ARGS__> \
	struct std::hash<x<##__VA_ARGS__>> \
	{ \
	public: \
		SOLID_INLINE std::size_t operator()(const x<T>& Value) const noexcept \
		{ \
			return GetTypeHash(Value); \
		} \
		\
	}; // struct std::hash<x<T>>

#define DEFINE_STD_HASH_TEMPLATED_CUSTOM_FUNC(x, FUNC, ...) \
	template <__VA_ARGS__> \
	struct std::hash<x<__VA_ARGS__>> \
	{ \
	public: \
		SOLID_INLINE std::size_t operator()(const x<T>& Value) const noexcept \
		{ \
			return FUNC(Value); \
		} \
		\
	}; // struct std::hash<x<T>>

DEFINE_STD_HASH(FName)
DEFINE_STD_HASH(FString);
DEFINE_STD_HASH(FStringView);
DEFINE_STD_HASH(FGameplayTag);


template <typename T>
struct std::hash<TObjectKey<T>>
{
public:
	SOLID_INLINE std::size_t operator()(const TObjectKey<T>& Value) const noexcept
	{
		return GetTypeHash(Value);
	}
	
};; // struct std::hash<TObjectKey<T>>

/*template <typename T>
struct std::hash<TObjectKey<typename T>>
{
public:
	SOLID_INLINE std::size_t operator()(const TObjectKey<T>& Value) const
	{
		return GetTypeHash(Value);
	}
};;*/

#endif // SOLID_MACROS_STANDARD_HASHING_H
