// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Engine/DataAsset.h"
#include "Misc/DataValidation.h"

#include "FlecsWorldInfoSettings.h"

#include "FlecsWorldSettingsAsset.generated.h"

UCLASS(BlueprintType, Blueprintable, CollapseCategories)
class UNREALFLECS_API UFlecsWorldSettingsAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UFlecsWorldSettingsAsset();
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ShowOnlyInnerProperties))
	FFlecsWorldSettingsInfo WorldSettings;

#if WITH_EDITOR

	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;

#endif // WITH_EDITOR

	virtual FPrimaryAssetId GetPrimaryAssetId() const override final;

}; // class UFlecsWorldSettingsAsset
