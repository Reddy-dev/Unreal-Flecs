// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "StructUtils/InstancedStruct.h"

#include "SolidMacros/Macros.h"
#include "Types/SolidNotNull.h"
#include "Types/SolidEnumSelector.h"

#include "Entities/FlecsId.h"
#include "FlecsGeneratorTermRef.h"

#include "FlecsQueryGeneratorInputType.generated.h"

struct FFlecsQueryBuilderView;

UENUM(BlueprintType)
enum class EFlecsQueryGeneratorReturnType : uint8
{
	String,
	FlecsId,
}; // enum class EFlecsQueryGeneratorReturnType

USTRUCT(BlueprintInternalUseOnly)
struct UNREALFLECS_API FFlecsQueryGeneratorInputType
{
	GENERATED_BODY()

	
public:
	virtual ~FFlecsQueryGeneratorInputType() = default;
	
	UPROPERTY()
	EFlecsQueryGeneratorReturnType ReturnType = EFlecsQueryGeneratorReturnType::String;
	
	virtual NO_DISCARD FString GetStringOutput() const
	{
		solid_checkf(false, TEXT("FFlecsQueryGeneratorInputType::GetStringOutput: Pure virtual function called on base class! Did you forget to override it?"));
		return FString();
	}
	
	virtual NO_DISCARD FFlecsId GetFlecsIdOutput(const TSolidNotNull<const UFlecsWorld*> InWorld) const
	{
		solid_checkf(false, TEXT("FFlecsQueryGeneratorInputType::GetFlecsIdOutput: Pure virtual function called on base class! Did you forget to override it?"));
		return FFlecsId();
	}
	
	virtual void CustomBuilderOutput(FFlecsQueryBuilderView& Builder, const TSolidNotNull<const UFlecsWorld*> InWorld) const
	{
		solid_checkf(false, TEXT("FFlecsQueryGeneratorInputType::CustomBuilderOutput: Pure virtual function called on base class! Did you forget to override it?"));
	}
	
	NO_DISCARD FFlecsTermRef GetTermRefOutput(const TSolidNotNull<const UFlecsWorld*> InWorld) const;

}; // struct FFlecsQueryGeneratorInputType

USTRUCT(BlueprintType, meta = (DisplayName = "Script Struct"))
struct UNREALFLECS_API FFlecsQueryGeneratorInputType_ScriptStruct final : public FFlecsQueryGeneratorInputType
{
	GENERATED_BODY()
	
public:
	FFlecsQueryGeneratorInputType_ScriptStruct()
	{
		ReturnType = EFlecsQueryGeneratorReturnType::FlecsId;
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Query Generator")
	TObjectPtr<const UScriptStruct> ScriptStruct;
	
	virtual NO_DISCARD FFlecsId GetFlecsIdOutput(const TSolidNotNull<const UFlecsWorld*> InWorld) const override;
	
}; // struct FFlecsQueryGeneratorInputType_ScriptStruct

USTRUCT(BlueprintType, meta = (DisplayName = "Script Enum"))
struct UNREALFLECS_API FFlecsQueryGeneratorInputType_ScriptEnum final : public FFlecsQueryGeneratorInputType
{
	GENERATED_BODY()
	
public:
	FFlecsQueryGeneratorInputType_ScriptEnum()
	{
		ReturnType = EFlecsQueryGeneratorReturnType::FlecsId;
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Query Generator")
	TObjectPtr<const UEnum> ScriptEnum;
	
	virtual NO_DISCARD FFlecsId GetFlecsIdOutput(const TSolidNotNull<const UFlecsWorld*> InWorld) const override;
	
}; // struct FFlecsQueryGeneratorInputType_ScriptEnum

USTRUCT(BlueprintType, meta = (DisplayName = "String"))
struct UNREALFLECS_API FFlecsQueryGeneratorInputType_String final : public FFlecsQueryGeneratorInputType
{
	GENERATED_BODY()
	
public:
	FFlecsQueryGeneratorInputType_String()
	{
		ReturnType = EFlecsQueryGeneratorReturnType::String;
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Query Generator")
	FString InputString;
	
	virtual NO_DISCARD FString GetStringOutput() const override;
	
}; // struct FFlecsQueryGeneratorInputType_String

USTRUCT(NotBlueprintType)
struct UNREALFLECS_API FFlecsQueryGeneratorInputType_CPPType final : public FFlecsQueryGeneratorInputType
{
	GENERATED_BODY()
	
public:
	FFlecsQueryGeneratorInputType_CPPType()
	{
		ReturnType = EFlecsQueryGeneratorReturnType::FlecsId;
	}
	
	UPROPERTY()
	FString SymbolString;
	
	virtual NO_DISCARD FFlecsId GetFlecsIdOutput(const TSolidNotNull<const UFlecsWorld*> InWorld) const override;
	
}; // struct FFlecsQueryGeneratorInputType_CPPType

USTRUCT(BlueprintType, meta = (DisplayName = "Flecs Id"))
struct UNREALFLECS_API FFlecsQueryGeneratorInputType_FlecsId final : public FFlecsQueryGeneratorInputType
{
	GENERATED_BODY()
	
public:
	FFlecsQueryGeneratorInputType_FlecsId()
	{
		ReturnType = EFlecsQueryGeneratorReturnType::FlecsId;
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Query Generator")
	FFlecsId FlecsId;
	
	virtual NO_DISCARD FFlecsId GetFlecsIdOutput(const TSolidNotNull<const UFlecsWorld*> InWorld) const override;
	
}; // struct FFlecsQueryGeneratorInputType_FlecsId

USTRUCT(BlueprintType, meta = (DisplayName = "Script Enum Constant"))
struct UNREALFLECS_API FFlecsQueryGeneratorInputType_ScriptEnumConstant final : public FFlecsQueryGeneratorInputType
{
	GENERATED_BODY()
	
public:
	FFlecsQueryGeneratorInputType_ScriptEnumConstant()
	{
		ReturnType = EFlecsQueryGeneratorReturnType::FlecsId;
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Query Generator")
	FSolidEnumSelector EnumValue;
	
	virtual NO_DISCARD FFlecsId GetFlecsIdOutput(const TSolidNotNull<const UFlecsWorld*> InWorld) const override;
	
}; // struct FFlecsQueryGeneratorInputType_ScriptEnumConstant


USTRUCT(BlueprintType, meta = (DisplayName = "Wildcard"))
struct UNREALFLECS_API FFlecsQueryGeneratorInputType_Wildcard final : public FFlecsQueryGeneratorInputType
{
	GENERATED_BODY()
	
public:
	FFlecsQueryGeneratorInputType_Wildcard()
	{
		ReturnType = EFlecsQueryGeneratorReturnType::FlecsId;
	}
	
	virtual NO_DISCARD FFlecsId GetFlecsIdOutput(const TSolidNotNull<const UFlecsWorld*> InWorld) const override
	{
		return flecs::Wildcard;
	}
	
}; // struct FFlecsQueryGeneratorInputType_Wildcard

USTRUCT(BlueprintType, meta = (DisplayName = "Any"))
struct UNREALFLECS_API FFlecsQueryGeneratorInputType_Any final : public FFlecsQueryGeneratorInputType
{
	GENERATED_BODY()
	
public:
	FFlecsQueryGeneratorInputType_Any()
	{
		ReturnType = EFlecsQueryGeneratorReturnType::FlecsId;
	}
	
	virtual NO_DISCARD FFlecsId GetFlecsIdOutput(const TSolidNotNull<const UFlecsWorld*> InWorld) const override
	{
		return flecs::Any;
	}
	
}; // struct FFlecsQueryGeneratorInputType_Any