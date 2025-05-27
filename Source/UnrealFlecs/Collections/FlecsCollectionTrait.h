// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Entities/FlecsEntityHandle.h"
#include "UObject/Object.h"
#include "FlecsCollectionTrait.generated.h"

class UFlecsWorld;
struct FFlecsCollectionBuilder;

UENUM(BlueprintType)
enum class EFlecsCollectionTraitType : uint8
{
	None = 0,
	NewEntity,
	SlotEntity,
}; // enum class EFlecsCollectionTraitType

UCLASS(Abstract, Blueprintable, EditInlineNew)
class UNREALFLECS_API UFlecsCollectionTrait : public UObject
{
	GENERATED_BODY()

public:
	UFlecsCollectionTrait();
	UFlecsCollectionTrait(const FObjectInitializer& ObjectInitializer);

	NO_DISCARD FORCEINLINE EFlecsCollectionTraitType GetTraitType() const
	{
		return TraitType;
	}

	virtual void Build(FFlecsCollectionBuilder& InBuilder) const
		PURE_VIRTUAL(UFlecsCollectionTrait::Build, );
	
	virtual void OnPrefabCreated(FFlecsEntityHandle& InEntityHandle, UFlecsWorld* InWorld) {}
	virtual void DestroyTemplate(UFlecsWorld* InWorld) {}

protected:
	UPROPERTY()
	EFlecsCollectionTraitType TraitType = EFlecsCollectionTraitType::None;
	

}; // class UFlecsCollectionTrait
