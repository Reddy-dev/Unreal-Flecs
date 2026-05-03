// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Observers/FlecsObserverObject.h"

#include "FlecsTickFunctionObservers.generated.h"

UCLASS(NotBlueprintable, BlueprintType)
class UNREALFLECS_API UFlecsAddTickFunctionObserver final : public UFlecsObserverObject
{
	GENERATED_BODY()
	
public:
	virtual void BuildObserver(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld, TFlecsObserverBuilder<>& InOutBuilder) const override;
	virtual void EachIterator(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld, flecs::iter& InIterator, const FFlecsId InIndex) override;
	
}; // class UFlecsAddTickFunctionObserver

UCLASS(NotBlueprintable, BlueprintType)
class UNREALFLECS_API UFlecsRemoveTickFunctionObserver final : public UFlecsObserverObject
{
	GENERATED_BODY()
	
public:
	virtual void BuildObserver(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld, TFlecsObserverBuilder<>& InOutBuilder) const override;
	virtual void EachIterator(const TSolidNotNull<UFlecsWorldInterfaceObject*> InWorld, flecs::iter& InIterator, const FFlecsId InIndex) override;
	
}; // class UFlecsRemoveTickFunctionObserver


