// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SolidMacros/Macros.h"
#include "Types/SolidEnumSelector.h"

#include "Entities/FlecsId.h"
#include "StructUtils/InstancedStruct.h"

#include "FlecsQueryGeneratorInputType.generated.h"

USTRUCT(BlueprintInternalUseOnly)
struct UNREALFLECS_API FFlecsQueryGeneratorInputType
{
	GENERATED_BODY()
	
	enum class EQueryReturnType : uint8
	{
		String,
		FlecsId,
		CustomBuilder
	}; // enum class EReturnType
	
	EQueryReturnType ReturnType = EQueryReturnType::String;
	
public:
	virtual ~FFlecsQueryGeneratorInputType() = default;
	
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
	
	virtual void CustomBuilderOutput(flecs::query_builder<>& Builder, const TSolidNotNull<const UFlecsWorld*> InWorld) const
	{
		solid_checkf(false, TEXT("FFlecsQueryGeneratorInputType::CustomBuilderOutput: Pure virtual function called on base class! Did you forget to override it?"));
	}

}; // struct FFlecsQueryGeneratorInputType

USTRUCT(BlueprintType, meta = (DisplayName = "Script Struct"))
struct UNREALFLECS_API FFlecsQueryGeneratorInputType_ScriptStruct : public FFlecsQueryGeneratorInputType
{
	GENERATED_BODY()
	
public:
	FFlecsQueryGeneratorInputType_ScriptStruct()
	{
		ReturnType = EQueryReturnType::FlecsId;
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Query Generator")
	TObjectPtr<const UScriptStruct> ScriptStruct;
	
	virtual NO_DISCARD FFlecsId GetFlecsIdOutput(const TSolidNotNull<const UFlecsWorld*> InWorld) const override;
	
}; // struct FFlecsQueryGeneratorInputType_ScriptStruct

USTRUCT(BlueprintType, meta = (DisplayName = "Script Enum"))
struct UNREALFLECS_API FFlecsQueryGeneratorInputType_ScriptEnum : public FFlecsQueryGeneratorInputType
{
	GENERATED_BODY()
	
public:
	FFlecsQueryGeneratorInputType_ScriptEnum()
	{
		ReturnType = EQueryReturnType::FlecsId;
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Query Generator")
	TObjectPtr<const UEnum> ScriptEnum;
	
	virtual NO_DISCARD FFlecsId GetFlecsIdOutput(const TSolidNotNull<const UFlecsWorld*> InWorld) const override;
	
}; // struct FFlecsQueryGeneratorInputType_ScriptEnum

USTRUCT(BlueprintType)
struct UNREALFLECS_API FFlecsQueryGeneratorInputType_String : public FFlecsQueryGeneratorInputType
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Query Generator")
	FString InputString;
	
	virtual NO_DISCARD FString GetStringOutput() const override;
	
}; // struct FFlecsQueryGeneratorInputType_String

USTRUCT(BlueprintType, meta = (DisplayName = "Flecs Id"))
struct UNREALFLECS_API FFlecsQueryGeneratorInputType_FlecsId : public FFlecsQueryGeneratorInputType
{
	GENERATED_BODY()
	
public:
	FFlecsQueryGeneratorInputType_FlecsId()
	{
		ReturnType = EQueryReturnType::FlecsId;
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Query Generator")
	FFlecsId FlecsId;
	
	virtual NO_DISCARD FFlecsId GetFlecsIdOutput(const TSolidNotNull<const UFlecsWorld*> InWorld) const override;
	
}; // struct FFlecsQueryGeneratorInputType_FlecsId

USTRUCT(BlueprintType, meta = (DisplayName = "Script Enum Constant"))
struct UNREALFLECS_API FFlecsQueryGeneratorInputType_ScriptEnumConstant : public FFlecsQueryGeneratorInputType
{
	GENERATED_BODY()
	
public:
	FFlecsQueryGeneratorInputType_ScriptEnumConstant()
	{
		ReturnType = EQueryReturnType::FlecsId;
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Query Generator")
	FSolidEnumSelector EnumValue;
	
	virtual NO_DISCARD FFlecsId GetFlecsIdOutput(const TSolidNotNull<const UFlecsWorld*> InWorld) const override;
	
}; // struct FFlecsQueryGeneratorInputType_ScriptEnumConstant


USTRUCT(BlueprintType, meta = (DisplayName = "Wildcard"))
struct UNREALFLECS_API FFlecsQueryGeneratorInputType_Wildcard : public FFlecsQueryGeneratorInputType
{
	GENERATED_BODY()
	
public:
	FFlecsQueryGeneratorInputType_Wildcard()
	{
		ReturnType = EQueryReturnType::FlecsId;
	}
	
	virtual NO_DISCARD FFlecsId GetFlecsIdOutput(const TSolidNotNull<const UFlecsWorld*> InWorld) const override
	{
		return flecs::Wildcard;
	}
	
}; // struct FFlecsQueryGeneratorInputType_Wildcard

USTRUCT(BlueprintType, meta = (DisplayName = "Any"))
struct UNREALFLECS_API FFlecsQueryGeneratorInputType_Any : public FFlecsQueryGeneratorInputType
{
	GENERATED_BODY()
	
public:
	FFlecsQueryGeneratorInputType_Any()
	{
		ReturnType = EQueryReturnType::FlecsId;
	}
	
	virtual NO_DISCARD FFlecsId GetFlecsIdOutput(const TSolidNotNull<const UFlecsWorld*> InWorld) const override
	{
		return flecs::Any;
	}
	
}; // struct FFlecsQueryGeneratorInputType_Any

USTRUCT(BlueprintType, meta = (DisplayName = "Pair"))
struct UNREALFLECS_API FFlecsQueryGeneratorInputType_Pair : public FFlecsQueryGeneratorInputType
{
	GENERATED_BODY()
	
public:
	FFlecsQueryGeneratorInputType_Pair()
	{
		ReturnType = EQueryReturnType::CustomBuilder;
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Query Generator")
	TInstancedStruct<FFlecsQueryGeneratorInputType> First;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flecs | Query Generator")
	TInstancedStruct<FFlecsQueryGeneratorInputType> Second;
	
	virtual void CustomBuilderOutput(flecs::query_builder<>& Builder, const TSolidNotNull<const UFlecsWorld*> InWorld) const override;
	
}; // struct FFlecsQueryGeneratorInputType_Pair


USTRUCT(BlueprintType, meta = (DisplayName = "Name"))
struct UNREALFLECS_API FFlecsQueryGeneratorInputType_WithNameComponent : public FFlecsQueryGeneratorInputType
{
	GENERATED_BODY()
	
public:
	FFlecsQueryGeneratorInputType_WithNameComponent()
	{
		ReturnType = EQueryReturnType::CustomBuilder;
	}
	
	virtual void CustomBuilderOutput(flecs::query_builder<>& Builder, const TSolidNotNull<const UFlecsWorld*> InWorld) const override;
	
}; // struct FFlecsQueryGeneratorInputType_WithNameComponent