// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Observers/FlecsObserverObject.h"

#include "FlecsSubEntityRecordNameObserver.generated.h"

UCLASS(NotBlueprintable, BlueprintType)
class UNREALFLECS_API UFlecsSubEntityRecordNameObserver : public UFlecsObserverObject
{
	GENERATED_BODY()

public:
	UFlecsSubEntityRecordNameObserver(const FObjectInitializer& ObjectInitializer);
	
	virtual void BuildObserver(const TSolidNotNull<UFlecsWorld*> InWorld, TFlecsObserverBuilder<>& InOutBuilder) const override;
	virtual void EachIterator(const TSolidNotNull<UFlecsWorld*> InWorld, flecs::iter& InIterator, const FFlecsId InIndex) override;
	
}; // class UFlecsSubEntityRecordNameObserver
