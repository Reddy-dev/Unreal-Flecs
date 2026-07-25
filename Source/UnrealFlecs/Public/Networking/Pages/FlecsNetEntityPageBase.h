// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "UObject/Object.h"
#include "Net/Core/PushModel/PushModelMacros.h"

#include "FlecsNetEntityPageBase.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class UNREALFLECS_API UFlecsNetEntityPageBase : public UObject
{
	GENERATED_BODY()
	REPLICATED_BASE_CLASS(UFlecsNetEntityPageBase);

public:
	
}; // class UFlecsNetEntityPageBase
