// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SolidMacros/Macros.h"

#include "Collections/FlecsCollectionInterface.h"

#include "FlecsViewerCollectionTypes.generated.h"

UCLASS(Abstract)
class UFlecsViewerCollectionBase : public UObject, public IFlecsCollectionInterface
{
	GENERATED_BODY()

public:
	virtual void BuildCollection(FFlecsCollectionBuilder& Builder) const override;
	
}; // class UFlecsViewerCollectionBase

UCLASS(BlueprintType, Blueprintable)
class UFlecsPlayerViewerCollection : public UFlecsViewerCollectionBase
{
	GENERATED_BODY()
	
public:
	virtual void BuildCollection(FFlecsCollectionBuilder& Builder) const override;
	
	virtual NO_DISCARD FInstancedStruct GetParametersType() const override;
	
	
}; // class UFlecsPlayerViewerCollection

UCLASS(BlueprintType, Blueprintable)
class UFlecsActorViewerCollection : public UFlecsViewerCollectionBase
{
	GENERATED_BODY()
	
public:
	virtual void BuildCollection(FFlecsCollectionBuilder& Builder) const override;
	
	virtual NO_DISCARD FInstancedStruct GetParametersType() const override;
	
	
}; // class UFlecsActorViewerCollection

UCLASS(BlueprintType, Blueprintable)
class UFlecsStreamSourceViewerCollection : public UFlecsViewerCollectionBase
{
	GENERATED_BODY()
	
public:
	virtual void BuildCollection(FFlecsCollectionBuilder& Builder) const override;
	
	virtual NO_DISCARD FInstancedStruct GetParametersType() const override;
	
	
}; // class UFlecsStreamSourceViewerCollection


