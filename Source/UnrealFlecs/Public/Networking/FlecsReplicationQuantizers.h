// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/** Leaves a replicated value unchanged. This is the default codec. */
struct FFlecsNoReplicationQuantizer
{
	static constexpr const TCHAR* Fingerprint = TEXT("None");

	template <typename T>
	static void Quantize(T&)
	{
	}
	
}; // struct FFlecsNoReplicationQuantizer

/** Opt-in decimal scalar quantizer. The maximum error is half of one step. */
template <int32 DecimalPlaces>
struct TFlecsScalarReplicationQuantizer
{
	static_assert(DecimalPlaces >= 0 && DecimalPlaces <= 9);
	static constexpr const TCHAR* Fingerprint = TEXT("ScalarDecimal");

	template <typename T>
	static void Quantize(T& InOutValue)
	{
		static_assert(std::is_arithmetic_v<T>, "The scalar quantizer requires an arithmetic value");
		const double Scale = FMath::Pow(10.0, DecimalPlaces);
		InOutValue = static_cast<T>(FMath::RoundToDouble(static_cast<double>(InOutValue) * Scale) / Scale);
	}

	static FString GetFingerprint()
	{
		return FString::Printf(TEXT("%s:%d"), Fingerprint, DecimalPlaces);
	}
	
}; // struct TFlecsScalarReplicationQuantizer

/** Opt-in decimal FVector quantizer. Each axis has half-step maximum error. */
template <int32 DecimalPlaces>
struct TFlecsVectorReplicationQuantizer
{
	static_assert(DecimalPlaces >= 0 && DecimalPlaces <= 9);
	static constexpr const TCHAR* Fingerprint = TEXT("VectorDecimal");

	static void Quantize(FVector& InOutValue)
	{
		const double Scale = FMath::Pow(10.0, DecimalPlaces);
		
		InOutValue.X = FMath::RoundToDouble(InOutValue.X * Scale) / Scale;
		InOutValue.Y = FMath::RoundToDouble(InOutValue.Y * Scale) / Scale;
		InOutValue.Z = FMath::RoundToDouble(InOutValue.Z * Scale) / Scale;
	}

	static FString GetFingerprint()
	{
		return FString::Printf(TEXT("%s:%d"), Fingerprint, DecimalPlaces);
	}
	
}; // struct TFlecsVectorReplicationQuantizer

/** Opt-in decimal FRotator quantizer. Angles are normalized before rounding. */
template <int32 DecimalPlaces>
struct TFlecsRotatorReplicationQuantizer
{
	static_assert(DecimalPlaces >= 0 && DecimalPlaces <= 9);
	static constexpr const TCHAR* Fingerprint = TEXT("RotatorDecimal");

	static void Quantize(FRotator& InOutValue)
	{
		InOutValue.Normalize();
		const double Scale = FMath::Pow(10.0, DecimalPlaces);
		InOutValue.Pitch = FMath::RoundToDouble(InOutValue.Pitch * Scale) / Scale;
		InOutValue.Yaw = FMath::RoundToDouble(InOutValue.Yaw * Scale) / Scale;
		InOutValue.Roll = FMath::RoundToDouble(InOutValue.Roll * Scale) / Scale;
	}

	static FString GetFingerprint()
	{
		return FString::Printf(TEXT("%s:%d"), Fingerprint, DecimalPlaces);
	}
	
}; // struct TFlecsRotatorReplicationQuantizer
