// Parser-only Unreal Flecs type-map declaration.

#pragma once

#include "Unreal.h"
#include "FlecsScriptClassComponent.h"
#include "FlecsScriptEnumComponent.h"
#include "FlecsScriptStructComponent.h"
#include "Standard/robin_hood.h"

struct FFlecsParserTypeMap
{
	template <typename... T>
	void emplace(T&&...)
	{
	}
};

struct FFlecsTypeMapComponent
{
	FFlecsParserTypeMap ScriptStructMap;
	FFlecsParserTypeMap ScriptClassMap;
	FFlecsParserTypeMap ScriptEnumMap;
};
