// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "FlecsTickingGroup.h"

#include "SolidMacros/Macros.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UnrealFlecsTickingGroup)

bool IsConvertibleToFlecsTickingGroup(const ETickingGroup InEngineTickingGroup)
{
	return InEngineTickingGroup == TG_PrePhysics
		|| InEngineTickingGroup == TG_DuringPhysics
		|| InEngineTickingGroup == TG_PostPhysics
		|| InEngineTickingGroup == TG_PostUpdateWork;
}

ETickingGroup ConvertFlecsTickingGroupToEngineTickingGroup(const EFlecsTickingGroup InFlecsTickingGroup)
{
	switch (InFlecsTickingGroup)
	{
		case EFlecsTickingGroup::PrePhysics:
			return TG_PrePhysics;
		case EFlecsTickingGroup::DuringPhysics:
			return TG_DuringPhysics;
		case EFlecsTickingGroup::PostPhysics:
			return TG_PostPhysics;
		case EFlecsTickingGroup::PostUpdateWork:
			return TG_PostUpdateWork;
		default: UNLIKELY_ATTRIBUTE
			solid_checkf(false, TEXT("ConvertFlecsTickingGroupToEngineTickingGroup: Invalid EFlecsTickingGroup value"));
			return TG_PrePhysics;
	}
}

EFlecsTickingGroup ConvertEngineTickingGroupToFlecsTickingGroup(const ETickingGroup InEngineTickingGroup)
{
	solid_check(IsConvertibleToFlecsTickingGroup(InEngineTickingGroup));
	
	switch (InEngineTickingGroup)
	{
		case TG_PrePhysics:
			return EFlecsTickingGroup::PrePhysics;
		case TG_DuringPhysics:
			return EFlecsTickingGroup::DuringPhysics;
		case TG_PostPhysics:
			return EFlecsTickingGroup::PostPhysics;
		case TG_PostUpdateWork:
			return EFlecsTickingGroup::PostUpdateWork;
		default: UNLIKELY_ATTRIBUTE
			solid_checkf(false, TEXT("ConvertEngineTickingGroupToFlecsTickingGroup: Invalid ETickingGroup value"));
			return EFlecsTickingGroup::PrePhysics;
	}
}
