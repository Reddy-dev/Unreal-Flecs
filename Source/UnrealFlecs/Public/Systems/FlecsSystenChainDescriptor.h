// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "FlecsSystenChainDescriptor.generated.h"

UENUM(BlueprintType)
enum class EFlecsSystemChainItemKind : uint8
{
	Nine
};

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsSystemChainItemDescriptor
{
	GENERATED_BODY()
	
public:
	
}; // struct FFlecsSystemChainItemDescriptor

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsSystemChainDescriptor
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Systems")
	FName ChainName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Systems")
	TArray<FName> SystemNames;
}; // struct FFlecsSystemChainDescriptor