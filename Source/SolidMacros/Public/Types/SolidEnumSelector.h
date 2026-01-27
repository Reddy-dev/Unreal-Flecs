// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SolidMacros/Macros.h"
#include "Concepts/SolidConcepts.h"

#include "SolidEnumSelector.generated.h"

/*
* @TODO: Meta Specifiers:
* Meta Specifiers:
 * AllowedEnums: Comma separated list of enum type names that are allowed.
 * DisallowedEnums: Comma separated list of enum type names that are disallowed.
 * LockEnumClass: If true, the enum class cannot be changed in the editor.
*/

/**
 * Taken from @FEnumSelector in the PCG Module.
 */
USTRUCT(BlueprintType)
struct SOLIDMACROS_API FSolidEnumSelector
{
	GENERATED_BODY()

public:
	FSolidEnumSelector() = default;

	// @TODO: fix this not having anough constraints
	template <typename TEnum>
	requires (std::is_enum<TEnum>::value)
	static NO_DISCARD FSolidEnumSelector Make(const TEnum InValue)
	{
		//using EnumType = decltype(InValue);
		static_assert(Solid::TStaticEnumConcept<TEnum>, "FSolidEnumSelector::Make<TEnum>: TEnum must be a UENUM type.");
		
		return FSolidEnumSelector(StaticEnum<TEnum>(), static_cast<int64>(InValue));
	}

	static NO_DISCARD FSolidEnumSelector Make(const UEnum* InClass, const int64 InValue)
	{
		return FSolidEnumSelector(InClass, InValue);
	}

	FORCEINLINE FSolidEnumSelector(const UEnum* InClass, const int64 InValue)
		: Class(InClass), Value(InValue)
	{
	}
	
	UPROPERTY(DisplayName="Enum Class")
	TObjectPtr<const UEnum> Class;

	UPROPERTY(DisplayName="Enum Value")
	int64 Value = 0;

	NO_DISCARD FText GetDisplayName() const;
	NO_DISCARD FString GetCultureInvariantDisplayName() const;
	
}; // struct FSolidEnumSelector
