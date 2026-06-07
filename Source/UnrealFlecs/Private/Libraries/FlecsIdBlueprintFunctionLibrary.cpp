// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Libraries/FlecsIdBlueprintFunctionLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsIdBlueprintFunctionLibrary)

FFlecsId UFlecsIdBlueprintFunctionLibrary::MakeFlecsId(const int32 Index, const int32 Generation)
{
	return FFlecsId(Index, Generation);
}

void UFlecsIdBlueprintFunctionLibrary::BreakFlecsId(const FFlecsId Id, int32& OutIndex, int32& OutGeneration)
{
	OutIndex = Id.GetIndex();
	OutGeneration = Id.GetGeneration();
}

FFlecsId UFlecsIdBlueprintFunctionLibrary::MakePairId(const FFlecsId First, const FFlecsId Second)
{
	return FFlecsId::MakePair(First, Second);
}

bool UFlecsIdBlueprintFunctionLibrary::BreakPairId(const FFlecsId PairId, FFlecsId& OutFirst, FFlecsId& OutSecond)
{
	if (PairId.IsPair())
	{
		OutFirst = PairId.GetFirst();
		OutSecond = PairId.GetSecond();
		return true;
	}
	else
	{
		OutFirst = FFlecsId::Null();
		OutSecond = FFlecsId::Null();
		return false;
	}
}

bool UFlecsIdBlueprintFunctionLibrary::EqualEqual_FlecsId(const FFlecsId A, const FFlecsId B)
{
	return A == B;
}

bool UFlecsIdBlueprintFunctionLibrary::NotEqual_FlecsId(const FFlecsId A, const FFlecsId B)
{
	return A != B;
}

FString UFlecsIdBlueprintFunctionLibrary::ToString_FlecsId(const FFlecsId Id)
{
	return Id.ToString();
}
