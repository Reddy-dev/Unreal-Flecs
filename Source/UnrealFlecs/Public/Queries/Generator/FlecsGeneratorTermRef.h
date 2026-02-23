// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Entities/FlecsId.h"

using FFlecsTermRef = TVariant<FFlecsId, FString>;
using FFlecsTermRefAtom_Internal = TVariant<FFlecsId, char*>;

namespace Unreal::Flecs::Queries
{
	NO_DISCARD FORCEINLINE FFlecsTermRefAtom_Internal ToTermRefAtom(const FFlecsTermRef& InTermRef)
	{
		if (InTermRef.IsType<FFlecsId>())
		{
			return FFlecsTermRefAtom_Internal(TInPlaceType<FFlecsId>(), InTermRef.Get<FFlecsId>());
		}
		else
		{
			const FString& StringRef = InTermRef.Get<FString>();
			
			FTCHARToUTF8 UTF8String(*StringRef);
			
			const int32 Length = UTF8String.Length();
			
			char* OwnedCharPtr = static_cast<char*>(FMemory::Malloc(Length + 1));
			solid_cassume(OwnedCharPtr != nullptr);
			
			FMemory::Memcpy(OwnedCharPtr, UTF8String.Get(), Length);
			OwnedCharPtr[Length] = '\0';
			
			return FFlecsTermRefAtom_Internal(TInPlaceType<char*>(), OwnedCharPtr);
		}
	}
	
	FORCEINLINE void FreeTermRefAtom(const FFlecsTermRefAtom_Internal& InTermRefAtom)
	{
		if (InTermRefAtom.IsType<char*>())
		{
			char* CharPtr = InTermRefAtom.Get<char*>();
			FMemory::Free(CharPtr);
		}
		else UNLIKELY_ATTRIBUTE
		{
			UNREACHABLE // if you are seeing this, you really fucked up
		}
	}
	
} // namespace Unreal::Flecs::Queries

