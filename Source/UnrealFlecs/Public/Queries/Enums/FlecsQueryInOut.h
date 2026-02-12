// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "flecs.h"

#include "CoreMinimal.h"
#include "FlecsQueryInOut.generated.h"

UENUM(BlueprintType)
enum class EFlecsQueryInOut : uint8
{
	/**< InOut for regular terms, In for shared terms */
	Default = flecs::InOutDefault,
	/**< Term is neither read nor written */
	None = flecs::InOutNone,
	/** Term is only read */
	Read = flecs::In,
	/**< Term is only written */
	Write = flecs::Out,
	/**< Term is both read and written */
	ReadWrite = flecs::InOut UMETA(DisplayName = "Read/Write"),
	/** Same as None + prevents term from triggering observers */
	Filter = flecs::InOutFilter
}; // enum class EFlecsQueryInOut

NO_DISCARD FORCEINLINE constexpr flecs::inout_kind_t ToFlecsInOut(EFlecsQueryInOut InOut)
{
	return static_cast<flecs::inout_kind_t>(InOut);
}