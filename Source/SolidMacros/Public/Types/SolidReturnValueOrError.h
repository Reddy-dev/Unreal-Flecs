// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include <memory>
#include <new>
#include <type_traits>
#include <utility>

#include "CoreMinimal.h"

#include "SolidMacros/Macros.h"

/**
 * Stores either a return value or an error without allocating or default constructing either type.
 *
 * Construct instances through FromValue or FromError. Accessing the inactive alternative is a
 * contract violation: it checks in non-shipping builds and becomes an optimizer assumption in
 * shipping builds through solid_cassumef.
 */
template <typename ValueType, typename ErrorType>
class TReturnValueOrError
{
	static_assert(!std::is_void_v<ValueType>, "TReturnValueOrError cannot store void as a value");
	static_assert(!std::is_void_v<ErrorType>, "TReturnValueOrError cannot store void as an error");
	static_assert(!std::is_reference_v<ValueType>, "TReturnValueOrError cannot store a value reference");
	static_assert(!std::is_reference_v<ErrorType>, "TReturnValueOrError cannot store an error reference");
	static_assert(std::is_destructible_v<ValueType>, "TReturnValueOrError value type must be destructible");
	static_assert(std::is_destructible_v<ErrorType>, "TReturnValueOrError error type must be destructible");

	struct FValueTag
	{
	}; // struct FValueTag

	struct FErrorTag
	{
	}; // struct FErrorTag

	union FStorage
	{
		constexpr FStorage()
		{
		}

		~FStorage()
		{
		}

		ValueType Value;
		ErrorType Error;
	}; // union FStorage

	FStorage Storage;
	bool bHasValue;

	template <typename... ArgTypes>
	FORCEINLINE explicit TReturnValueOrError(FValueTag, ArgTypes&&... InArgs)
		: bHasValue(true)
	{
		std::construct_at(std::addressof(Storage.Value), Forward<ArgTypes>(InArgs)...);
	}

	template <typename... ArgTypes>
	FORCEINLINE explicit TReturnValueOrError(FErrorTag, ArgTypes&&... InArgs)
		: bHasValue(false)
	{
		std::construct_at(std::addressof(Storage.Error), Forward<ArgTypes>(InArgs)...);
	}

	FORCEINLINE void DestroyActive()
	{
		if (bHasValue)
		{
			std::destroy_at(std::addressof(Storage.Value));
		}
		else
		{
			std::destroy_at(std::addressof(Storage.Error));
		}
	}

public:
	TReturnValueOrError() = delete;

	template <typename... ArgTypes>
	requires std::is_constructible_v<ValueType, ArgTypes&&...>
	NO_DISCARD static FORCEINLINE TReturnValueOrError FromValue(ArgTypes&&... InArgs)
	{
		return TReturnValueOrError(FValueTag{}, Forward<ArgTypes>(InArgs)...);
	}

	template <typename... ArgTypes>
	requires std::is_constructible_v<ErrorType, ArgTypes&&...>
	NO_DISCARD static FORCEINLINE TReturnValueOrError FromError(ArgTypes&&... InArgs)
	{
		return TReturnValueOrError(FErrorTag{}, Forward<ArgTypes>(InArgs)...);
	}

	FORCEINLINE TReturnValueOrError(const TReturnValueOrError& Other)
		requires(std::is_copy_constructible_v<ValueType> && std::is_copy_constructible_v<ErrorType>)
		: bHasValue(Other.bHasValue)
	{
		if (bHasValue)
		{
			std::construct_at(std::addressof(Storage.Value), Other.Storage.Value);
		}
		else
		{
			std::construct_at(std::addressof(Storage.Error), Other.Storage.Error);
		}
	}

	TReturnValueOrError(const TReturnValueOrError& Other)
		requires(!std::is_copy_constructible_v<ValueType> || !std::is_copy_constructible_v<ErrorType>) = delete;

	FORCEINLINE TReturnValueOrError(TReturnValueOrError&& Other) noexcept(
		std::is_nothrow_move_constructible_v<ValueType> && std::is_nothrow_move_constructible_v<ErrorType>)
		requires(std::is_move_constructible_v<ValueType> && std::is_move_constructible_v<ErrorType>)
		: bHasValue(Other.bHasValue)
	{
		if (bHasValue)
		{
			std::construct_at(std::addressof(Storage.Value), MoveTemp(Other.Storage.Value));
		}
		else
		{
			std::construct_at(std::addressof(Storage.Error), MoveTemp(Other.Storage.Error));
		}
	}

	TReturnValueOrError(TReturnValueOrError&& Other)
		requires(!std::is_move_constructible_v<ValueType> || !std::is_move_constructible_v<ErrorType>) = delete;

	FORCEINLINE TReturnValueOrError& operator=(const TReturnValueOrError& Other)
		requires(
			std::is_copy_constructible_v<ValueType> && std::is_copy_assignable_v<ValueType>
			&& std::is_copy_constructible_v<ErrorType> && std::is_copy_assignable_v<ErrorType>)
	{
		if (this == std::addressof(Other))
		{
			return *this;
		}

		if (bHasValue == Other.bHasValue)
		{
			if (bHasValue)
			{
				Storage.Value = Other.Storage.Value;
			}
			else
			{
				Storage.Error = Other.Storage.Error;
			}
		}
		else
		{
			DestroyActive();
			bHasValue = Other.bHasValue;

			if (bHasValue)
			{
				std::construct_at(std::addressof(Storage.Value), Other.Storage.Value);
			}
			else
			{
				std::construct_at(std::addressof(Storage.Error), Other.Storage.Error);
			}
		}

		return *this;
	}

	TReturnValueOrError& operator=(const TReturnValueOrError& Other)
		requires(
			!std::is_copy_constructible_v<ValueType> || !std::is_copy_assignable_v<ValueType>
			|| !std::is_copy_constructible_v<ErrorType> || !std::is_copy_assignable_v<ErrorType>) = delete;

	FORCEINLINE TReturnValueOrError& operator=(TReturnValueOrError&& Other) noexcept(
		std::is_nothrow_move_constructible_v<ValueType> && std::is_nothrow_move_assignable_v<ValueType>
		&& std::is_nothrow_move_constructible_v<ErrorType> && std::is_nothrow_move_assignable_v<ErrorType>)
		requires(
			std::is_move_constructible_v<ValueType> && std::is_move_assignable_v<ValueType>
			&& std::is_move_constructible_v<ErrorType> && std::is_move_assignable_v<ErrorType>)
	{
		if (this == std::addressof(Other))
		{
			return *this;
		}

		if (bHasValue == Other.bHasValue)
		{
			if (bHasValue)
			{
				Storage.Value = MoveTemp(Other.Storage.Value);
			}
			else
			{
				Storage.Error = MoveTemp(Other.Storage.Error);
			}
		}
		else
		{
			DestroyActive();
			bHasValue = Other.bHasValue;

			if (bHasValue)
			{
				std::construct_at(std::addressof(Storage.Value), MoveTemp(Other.Storage.Value));
			}
			else
			{
				std::construct_at(std::addressof(Storage.Error), MoveTemp(Other.Storage.Error));
			}
		}

		return *this;
	}

	TReturnValueOrError& operator=(TReturnValueOrError&& Other)
		requires(
			!std::is_move_constructible_v<ValueType> || !std::is_move_assignable_v<ValueType>
			|| !std::is_move_constructible_v<ErrorType> || !std::is_move_assignable_v<ErrorType>) = delete;

	FORCEINLINE ~TReturnValueOrError()
	{
		if constexpr (!std::is_trivially_destructible_v<ValueType> || !std::is_trivially_destructible_v<ErrorType>)
		{
			DestroyActive();
		}
	}

	NO_DISCARD FORCEINLINE bool HasValue() const
	{
		return bHasValue;
	}

	NO_DISCARD FORCEINLINE bool HasError() const
	{
		return !bHasValue;
	}

	NO_DISCARD FORCEINLINE explicit operator bool() const
	{
		return bHasValue;
	}

	NO_DISCARD FORCEINLINE ValueType& GetValue() &
	{
		solid_cassumef(bHasValue, TEXT("TReturnValueOrError does not contain a return value"));
		return Storage.Value;
	}

	NO_DISCARD FORCEINLINE const ValueType& GetValue() const&
	{
		solid_cassumef(bHasValue, TEXT("TReturnValueOrError does not contain a return value"));
		return Storage.Value;
	}

	NO_DISCARD FORCEINLINE ValueType&& GetValue() &&
	{
		solid_cassumef(bHasValue, TEXT("TReturnValueOrError does not contain a return value"));
		return MoveTemp(Storage.Value);
	}

	NO_DISCARD FORCEINLINE ErrorType& GetError() &
	{
		solid_cassumef(!bHasValue, TEXT("TReturnValueOrError does not contain an error"));
		return Storage.Error;
	}

	NO_DISCARD FORCEINLINE const ErrorType& GetError() const&
	{
		solid_cassumef(!bHasValue, TEXT("TReturnValueOrError does not contain an error"));
		return Storage.Error;
	}

	NO_DISCARD FORCEINLINE ErrorType&& GetError() &&
	{
		solid_cassumef(!bHasValue, TEXT("TReturnValueOrError does not contain an error"));
		return MoveTemp(Storage.Error);
	}

	NO_DISCARD FORCEINLINE ValueType* TryGetValue()
	{
		return bHasValue ? std::addressof(Storage.Value) : nullptr;
	}

	NO_DISCARD FORCEINLINE const ValueType* TryGetValue() const
	{
		return bHasValue ? std::addressof(Storage.Value) : nullptr;
	}

	NO_DISCARD FORCEINLINE ErrorType* TryGetError()
	{
		return bHasValue ? nullptr : std::addressof(Storage.Error);
	}

	NO_DISCARD FORCEINLINE const ErrorType* TryGetError() const
	{
		return bHasValue ? nullptr : std::addressof(Storage.Error);
	}
}; // class TReturnValueOrError
