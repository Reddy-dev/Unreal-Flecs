// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Engine/EngineBaseTypes.h"
#include "GameplayTagContainer.h"

#include "FlecsTickFunction.generated.h"

class UFlecsWorld;

USTRUCT()
struct UNREALFLECS_API FFlecsTickFunction : public FTickFunction
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Flecs")
	FGameplayTag TickTypeTag;

	UPROPERTY(Transient)
	TObjectPtr<UFlecsWorld> OwningWorld;

	virtual void ExecuteTick(float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread,
		const FGraphEventRef& MyCompletionGraphEvent) override;

	virtual FString DiagnosticMessage() override;

	
	
}; // struct FFlecsTickFunction

template<>
struct TStructOpsTypeTraits<FFlecsTickFunction> : public TStructOpsTypeTraitsBase2<FFlecsTickFunction>
{
	enum
	{
		WithCopy = false,
	}; // enum
	
}; // struct TStructOpsTypeTraits<FFlecsTickFunction>
