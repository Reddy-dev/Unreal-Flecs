// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "UObject/Object.h"

#include "Net/Core/PushModel/PushModelMacros.h"

#include "FlecsNetEntityProxyBase.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class UNREALFLECS_API UFlecsNetEntityProxyBase : public UObject
{
	GENERATED_BODY()
	REPLICATED_BASE_CLASS(UFlecsNetEntityProxyBase);

public:
	
}; // class UFlecsNetEntityProxyBase
