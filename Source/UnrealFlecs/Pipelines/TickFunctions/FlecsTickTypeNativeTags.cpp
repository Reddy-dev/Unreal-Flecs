// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "FlecsTickTypeNativeTags.h"

UE_DEFINE_GAMEPLAY_TAG(FlecsTickType_Root, "Flecs.TickType");
UE_DEFINE_GAMEPLAY_TAG(FlecsTickType_MainLoop, "Flecs.TickType.MainLoop");
UE_DEFINE_GAMEPLAY_TAG(FlecsTickType_PrePhysics, "Flecs.TickType.PrePhysics");
UE_DEFINE_GAMEPLAY_TAG(FlecsTickType_DuringPhysics, "Flecs.TickType.DuringPhysics");
UE_DEFINE_GAMEPLAY_TAG(FlecsTickType_PostPhysics, "Flecs.TickType.PostPhysics");
UE_DEFINE_GAMEPLAY_TAG(FlecsTickType_PostUpdateWork, "Flecs.TickType.PostUpdateWork");