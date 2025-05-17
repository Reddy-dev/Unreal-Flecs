﻿// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "FlecsComponentCollectionObject.h"
#include "Worlds/FlecsWorldSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsComponentCollectionObject)

UFlecsComponentCollectionObject::UFlecsComponentCollectionObject()
{
}

UFlecsComponentCollectionObject::UFlecsComponentCollectionObject(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFlecsComponentCollectionObject::Apply()
{
	BP_OnApply();
}
