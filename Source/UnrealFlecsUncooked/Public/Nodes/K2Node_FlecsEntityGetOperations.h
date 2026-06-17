// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Nodes/Base/K2Node_FlecsEntityGetBase.h"

#include "K2Node_FlecsEntityGetOperations.generated.h"

UCLASS()
class UNREALFLECSUNCOOKED_API UK2Node_FlecsEntityGetOperation : public UK2Node_FlecsEntityGetBase
{
	GENERATED_BODY()

public:
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

protected:
	virtual bool IsPairNode() const override { return false; }
	virtual bool IsReferenceNode() const override { return false; }
};

UCLASS()
class UNREALFLECSUNCOOKED_API UK2Node_FlecsEntityGetRefOperation : public UK2Node_FlecsEntityGetBase
{
	GENERATED_BODY()

public:
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

protected:
	virtual bool IsPairNode() const override { return false; }
	virtual bool IsReferenceNode() const override { return true; }
};

UCLASS()
class UNREALFLECSUNCOOKED_API UK2Node_FlecsEntityGetPairOperation  : public UK2Node_FlecsEntityGetBase
{
	GENERATED_BODY()

public:
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

protected:
	virtual bool IsPairNode() const override { return true; }
	virtual bool IsReferenceNode() const override { return false; }
};

UCLASS()
class UNREALFLECSUNCOOKED_API UK2Node_FlecsEntityGetPairRefOperation : public UK2Node_FlecsEntityGetBase
{
	GENERATED_BODY()

public:
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

protected:
	virtual bool IsPairNode() const override { return true; }
	virtual bool IsReferenceNode() const override { return true; }
};

UCLASS()
class UNREALFLECSUNCOOKED_API UK2Node_FlecsEntityGetUntypedComponentRefOperation : public UK2Node_FlecsEntityGetBase
{
	GENERATED_BODY()

public:
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

protected:
	virtual bool IsPairNode() const override { return false; }
	virtual bool IsReferenceNode() const override { return false; }
	virtual bool IsUntypedComponentRefNode() const override { return true; }
};

UCLASS()
class UNREALFLECSUNCOOKED_API UK2Node_FlecsEntityGetPairUntypedComponentRefOperation : public UK2Node_FlecsEntityGetBase
{
	GENERATED_BODY()

public:
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

protected:
	virtual bool IsPairNode() const override { return true; }
	virtual bool IsReferenceNode() const override { return false; }
	virtual bool IsUntypedComponentRefNode() const override { return true; }
};
