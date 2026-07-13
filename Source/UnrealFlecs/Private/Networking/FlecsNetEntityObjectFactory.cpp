// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/FlecsNetEntityObjectFactory.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsNetEntityObjectFactory)

namespace UE::Flecs::Net::Private
{
	static FName NetEntityObjectFactoryName = TEXT("FlecsNetEntityFactory");
	
} // namespace UE::Flecs::Net::Private

FName UFlecsNetEntityObjectFactory::GetFactoryName()
{
	return UE::Flecs::Net::Private::NetEntityObjectFactoryName;
}
