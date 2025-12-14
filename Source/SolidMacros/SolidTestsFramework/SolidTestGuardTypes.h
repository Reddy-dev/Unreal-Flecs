// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Runtime/Core/Internal/Misc/PlayInEditorLoadingScope.h"

namespace Solid::Tests
{
	struct SOLIDMACROS_API FGWorldGuard final : FNoncopyable
	{
		FGWorldGuard()
			: OldWorld{GWorld}
		{
		}

		~FGWorldGuard()
		{
			GWorld = OldWorld;
		}

		UWorld* OldWorld;
	
	}; // struct FGWorldGuard

	struct SOLIDMACROS_API FNetDriverTickRateAdjuster final : public FNoncopyable
	{
		FNetDriverTickRateAdjuster();

		~FNetDriverTickRateAdjuster();

	private:
		FDelegateHandle Handle;

		void OnNetDriverCreated(UWorld* InWorld, UNetDriver* InNetDriver);

	}; // struct FNetDriverTickRateAdjuster

	struct SOLIDMACROS_API FGPlayInEditorIDGuard final : UE::Core::Private::FPlayInEditorLoadingScope
	{
		explicit FGPlayInEditorIDGuard(const int32 PlayInEditorID)
			: FPlayInEditorLoadingScope(PlayInEditorID)
		{
		}
		
	}; // struct FGPlayInEditorIDGuard
	
} // namespace Solid::Tests