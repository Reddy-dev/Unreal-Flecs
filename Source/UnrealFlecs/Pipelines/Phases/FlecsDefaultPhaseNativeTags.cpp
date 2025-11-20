// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "FlecsDefaultPhaseNativeTags.h"

const FFlecsDefaultPhaseNativeTags& FFlecsDefaultPhaseNativeTags::Get()
{
	static FFlecsDefaultPhaseNativeTags DefaultPhaseTags;
	
	return DefaultPhaseTags;
}
