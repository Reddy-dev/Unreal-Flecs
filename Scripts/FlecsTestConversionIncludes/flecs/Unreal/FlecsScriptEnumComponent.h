// Parser-only Unreal Flecs component declaration.

#pragma once

#include "Unreal.h"

struct FFlecsScriptEnumComponent
{
	static UEnum* StaticEnum() { return nullptr; }
	FFlecsScriptEnumComponent(UEnum* = nullptr) {}
	UEnum* ScriptEnum = nullptr;
};
