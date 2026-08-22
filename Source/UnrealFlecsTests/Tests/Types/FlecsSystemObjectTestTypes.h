// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Systems/FlecsSystemObject.h"

#include "FlecsSystemObjectTestTypes.generated.h"

UCLASS()
class UNREALFLECSTESTS_API UFlecsStartsDisabledSystemTestObject final : public UFlecsSystemObject
{
	GENERATED_BODY()

public:
	UFlecsStartsDisabledSystemTestObject()
	{
		bStartsDisabled = true;
	}

	NO_DISCARD int32 GetRunCount() const
	{
		return RunCount;
	}

	virtual void BuildSystem(const TSolidNotNull<const UFlecsWorldInterfaceObject*>,
		TFlecsSystemBuilder<>& InBuilder) const override
	{
		InBuilder.Phase(EFlecsPhaseType::OnUpdate);
	}

	virtual void RunIterator(const TSolidNotNull<UFlecsWorldInterfaceObject*>,
		flecs::iter& InIterator) override
	{
		while (InIterator.next())
		{
			++RunCount;
		}
	}

	virtual bool ShouldAutoRegisterFromCDO() const override
	{
		return false;
	}

	virtual bool ShouldRegisterWithModule() const override
	{
		return false;
	}

private:
	int32 RunCount = 0;

}; // class UFlecsStartsDisabledSystemTestObject
