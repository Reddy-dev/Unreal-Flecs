// Parser-only Solid concept declarations for the Flecs test converter.
// Generated tests never include this directory.

#pragma once

#include "Unreal.h"

namespace Solid
{
	template <typename T>
	concept TVariantStructConcept = false;

	template <typename T>
	concept TScriptStructConcept = false;

	template <typename T>
	concept TStaticClassConcept = false;

	template <typename T>
	concept TStaticEnumConcept = false;

	template <typename T>
	inline constexpr bool IsScriptStruct()
	{
		return false;
	}

	template <typename T>
	inline constexpr bool IsStaticClass()
	{
		return false;
	}

	template <typename T>
	inline constexpr bool IsStaticEnum()
	{
		return false;
	}
}
