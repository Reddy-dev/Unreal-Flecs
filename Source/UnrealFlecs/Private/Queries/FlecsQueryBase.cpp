// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Queries/FlecsQueryBase.h"

#include "Queries/FlecsQuery.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsQueryBase)

FFlecsQueryBase::operator FFlecsQuery() const
{
	return FFlecsQuery(QueryBase);
}
