// Parser-only Unreal Flecs component declaration.

#pragma once

#include "Unreal.h"

struct FFlecsScriptClassComponent
{
	static UClass* StaticClass() { return nullptr; }
	FFlecsScriptClassComponent(UClass* = nullptr) {}
	UClass* ScriptClass = nullptr;
};
