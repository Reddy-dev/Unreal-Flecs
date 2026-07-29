// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SolidMacros/Macros.h"
#include "Trace/Analyzer.h"

namespace TraceServices
{
	class IAnalysisSession;
}

namespace UE::Flecs::RewindDebugger
{
	class FProvider;

	bool ValidateSnapshotPayload(const FString& InSnapshot, uint64 InExpectedEntityId);

	class FAnalyzer final : public UE::Trace::IAnalyzer
	{
	public:
		FAnalyzer(TraceServices::IAnalysisSession& InSession, FProvider& InProvider);

	private:
		virtual void OnAnalysisBegin(const FOnAnalysisContext& InContext) override;
		virtual bool OnEvent(
			uint16 InRouteId,
			EStyle InStyle,
			const FOnEventContext& InContext) override;

		enum : uint16
		{
			RouteId_EntityState,
			RouteId_EntityEnd
		};

		TraceServices::IAnalysisSession& Session;
		FProvider& Provider;
	};
} // namespace UE::Flecs::RewindDebugger
