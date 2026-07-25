// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "UObject/Object.h"
#include "Net/Core/PushModel/PushModelMacros.h"

#include "FlecsIrisFastArraySerializer.h"

#include "FlecsNetEntityPageBase.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class UNREALFLECS_API UFlecsNetEntityPageBase : public UObject
{
	GENERATED_BODY()
	REPLICATED_BASE_CLASS(UFlecsNetEntityPageBase)

public:
	virtual bool IsSupportedForNetworking() const override
	{
		return true;
	}
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Fragments, 
		UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
	
	UPROPERTY(ReplicatedUsing = OnRep_EntityPage)
	FFlecsNetEntityPageArray EntityPage;
	
	UFUNCTION()
	void OnRep_EntityPage();
	
}; // class UFlecsNetEntityPageBase
