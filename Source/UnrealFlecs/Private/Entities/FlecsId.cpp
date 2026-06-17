// Elie Wiese-Namir © 2025. All Rights Reserved.

#include "Entities/FlecsId.h"

#include "Worlds/FlecsWorldInterfaceObject.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsId)

const ecs_type_info_t* FFlecsId::GetTypeInfo(const TSolidNotNull<const UFlecsWorldInterfaceObject*> World) const
{
	return GetTypeInfo(World->GetNativeFlecsWorld());
}

FString FFlecsId::ToString() const
{
	if (GetId() == 0)
	{
		return TEXT("Null Id");
	}
	
	if (IsPair())
	{
		return FString::Printf(TEXT("First: %d, Second: %d"), GetFirst().GetIndex(), GetSecond().GetIndex());
	}
	
	return FString::Printf(TEXT("Index: %d, Generation: %d"), GetIndex(), GetGeneration());
}

bool FFlecsId::ImportTextItem(const TCHAR*& Buffer, int32 PortFlags, UObject* Parent, FOutputDevice* ErrorText)
{
	if (!Buffer)
	{
		return false;
	}

	const TCHAR* Cursor = Buffer;
	while (FChar::IsWhitespace(*Cursor))
	{
		++Cursor;
	}

	const bool bWrapped = *Cursor == TEXT('(');
	if (bWrapped)
	{
		++Cursor;
	}

	if (FCString::Strnicmp(Cursor, TEXT("FlecsId="), 8) == 0)
	{
		Cursor += 8;
	}
	else if (FCString::Strnicmp(Cursor, TEXT("Id="), 3) == 0)
	{
		Cursor += 3;
	}

	TCHAR* ParseEnd = nullptr;
	const uint64 ImportedId = FCString::Strtoui64(Cursor, &ParseEnd, 10);
	if (ParseEnd == Cursor)
	{
		return false;
	}

	if (bWrapped)
	{
		if (*ParseEnd != TEXT(')'))
		{
			return false;
		}
		++ParseEnd;
	}
	else if (*ParseEnd != TEXT('\0') &&
		*ParseEnd != TEXT(',') &&
		*ParseEnd != TEXT(')') &&
		!FChar::IsWhitespace(*ParseEnd))
	{
		return false;
	}

	Id = std::bit_cast<int64>(ImportedId);
	Buffer = ParseEnd;
	return true;
}

bool FFlecsId::ExportTextItem(FString& ValueStr, const FFlecsId& DefaultValue, UObject* Parent, int32 PortFlags,
	UObject* ExportRootScope) const
{
	ValueStr += FString::Printf(TEXT("FlecsId=%llu"), GetId());
	return true;
}
