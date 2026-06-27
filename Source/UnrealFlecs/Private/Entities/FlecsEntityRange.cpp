// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Entities/FlecsEntityRange.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsEntityRange)

UFlecsEntityRange::UFlecsEntityRange(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlecsEntityRange::SetNativeEntityRange(const ecs_entity_range_t* InRange, const FName& InRangeName)
{
	solid_cassume(InRange);
	NativeRange = InRange;
	RangeName = InRangeName;
}

void UFlecsEntityRange::InvalidateNativeEntityRange()
{
	NativeRange = nullptr;
}

uint32 UFlecsEntityRange::GetMinimum() const
{
	solid_cassume(NativeRange);
	return NativeRange->min;
}

uint32 UFlecsEntityRange::GetMaximum() const
{
	solid_cassume(NativeRange);
	return NativeRange->max;
}

int32 UFlecsEntityRange::K2_GetMinimum() const
{
	solid_cassume(NativeRange);
	return NativeRange->min;
}

int32 UFlecsEntityRange::K2_GetMaximum() const
{
	solid_cassume(NativeRange);
	return NativeRange->max;
}

FFlecsId UFlecsEntityRange::GetCurrentId() const
{
	solid_cassume(NativeRange);
	return FFlecsId(NativeRange->cur);
}
