// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "TraceServices/ModuleService.h"

namespace UE::Flecs::RewindDebugger
{
	class FTraceModule final : public TraceServices::IModule
	{
	public:
		virtual void GetModuleInfo(TraceServices::FModuleInfo& OutModuleInfo) override;
		virtual void OnAnalysisBegin(TraceServices::IAnalysisSession& InSession) override;
		virtual void GetLoggers(TArray<const TCHAR*>& OutLoggers) override;
		virtual const TCHAR* GetCommandLineArgument() override;
	};
} // namespace UE::Flecs::RewindDebugger
