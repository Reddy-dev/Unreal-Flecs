// Solstice Games © 2024. All Rights Reserved.

#pragma once


#include "Templates/SubclassOf.h"

#include "Standard/Hashing.h"

#include "FlecsScriptClassComponent.generated.h"

USTRUCT(BlueprintType)
struct FLECSLIBRARY_API FFlecsScriptClassComponent
{
    GENERATED_BODY()

    NO_DISCARD FORCEINLINE friend uint32 GetTypeHash(const FFlecsScriptClassComponent& InScriptStructComponent)
    {
        return GetTypeHash(InScriptStructComponent.ScriptClass);
    }

    NO_DISCARD FORCEINLINE bool UEOpEquals(const FFlecsScriptClassComponent& Other) const
    {
        return ScriptClass == Other.ScriptClass;
    }

    FORCEINLINE operator UClass*() const { return ScriptClass.Get(); }
    
    FORCEINLINE FFlecsScriptClassComponent(UClass* InScriptStruct = nullptr)
        : ScriptClass(InScriptStruct) {}
    FORCEINLINE FFlecsScriptClassComponent(const UClass* InScriptStruct)
        : ScriptClass(const_cast<UClass*>(InScriptStruct)) {}
    FORCEINLINE FFlecsScriptClassComponent(const FFlecsScriptClassComponent& InScriptStructComponent)
        : ScriptClass(InScriptStructComponent.ScriptClass) {}

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs")
    TSubclassOf<UObject> ScriptClass;
    
}; // struct FFlecsScriptClassComponent

DEFINE_STD_HASH(FFlecsScriptClassComponent)
