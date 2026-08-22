// Parser-only Unreal Flecs component declaration.

#pragma once

#include "Unreal.h"

struct FFlecsScriptStructComponent
{
	static UScriptStruct* StaticStruct() { return nullptr; }
	FFlecsScriptStructComponent(UScriptStruct* = nullptr) {}
	UScriptStruct* ScriptStruct = nullptr;
};
