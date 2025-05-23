﻿// Solstice Games © 2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Standard/Hashing.h"
#include "FlecsScriptClassComponent.generated.h"

USTRUCT(BlueprintType)
struct FLECSLIBRARY_API FFlecsScriptClassComponent
{
    GENERATED_BODY()

    NO_DISCARD FORCEINLINE friend uint32 GetTypeHash(const FFlecsScriptClassComponent& InScriptClassComponent)
    {
        return GetTypeHash(InScriptClassComponent.ScriptClass);
    }

    NO_DISCARD FORCEINLINE friend bool operator==(const FFlecsScriptClassComponent& Lhs, const FFlecsScriptClassComponent& Rhs)
    {
        return Lhs.ScriptClass == Rhs.ScriptClass;
    }

    NO_DISCARD FORCEINLINE friend bool operator!=(const FFlecsScriptClassComponent& Lhs, const FFlecsScriptClassComponent& Rhs)
    {
        return !(Lhs == Rhs);
    }

    FORCEINLINE operator TSubclassOf<UObject>() const { return ScriptClass.Get(); }
    
    FORCEINLINE FFlecsScriptClassComponent(const TSubclassOf<UObject>& InScriptClass = nullptr) : ScriptClass(InScriptClass) {}

    template <typename T>
    NO_DISCARD FORCEINLINE TSubclassOf<T> Get() const
    {
        return Cast<T>(ScriptClass.Get());
    }

    template <typename T>
    NO_DISCARD FORCEINLINE TSubclassOf<T> GetChecked() const
    {
        return CastChecked<T>(ScriptClass.Get());
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs")
    TSubclassOf<UObject> ScriptClass;
    
}; // struct FFlecsScriptClassComponent

DEFINE_STD_HASH(FFlecsScriptClassComponent)

