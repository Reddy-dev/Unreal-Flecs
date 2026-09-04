// Parser-only Unreal build configuration for the Flecs test converter.
//
// libclang does not receive Unreal Build Tool's generated command line. These
// values model the editor-development target solely so it can parse flecs.h;
// they are not used by the generated test build.

#pragma once

#define UE_BUILD_DEBUG 0
#define UE_BUILD_DEVELOPMENT 1
#define UE_BUILD_TEST 0
#define UE_BUILD_SHIPPING 0
#define UE_BUILD_DEBUG_WITH_DEBUGGAME 0

#define WITH_EDITOR 1
#define WITH_EDITORONLY_DATA 1
#define WITH_ENGINE 1
#define WITH_UNREAL_DEVELOPER_TOOLS 1
#define WITH_PLUGIN_SUPPORT 1

#define IS_MONOLITHIC 0
#define IS_PROGRAM 0

#define PLATFORM_WINDOWS 1
#define PLATFORM_64BITS 1
#define DO_CHECK 1

#define LIKELY_IF(Condition) (Condition)
#define UNLIKELY_IF(Condition) (Condition)
