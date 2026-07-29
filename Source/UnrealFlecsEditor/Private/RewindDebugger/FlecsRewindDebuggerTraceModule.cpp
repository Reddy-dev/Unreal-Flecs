// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "RewindDebugger/FlecsRewindDebuggerTraceModule.h"

#include "RewindDebugger/FlecsRewindDebuggerAnalyzer.h"
#include "RewindDebugger/FlecsRewindDebuggerProvider.h"
#include "TraceServices/Model/AnalysisSession.h"

namespace UE::Flecs::RewindDebugger
{
	void FTraceModule::GetModuleInfo(TraceServices::FModuleInfo& OutModuleInfo)
	{
		OutModuleInfo.Name = TEXT("FlecsRewindDebugger");
		OutModuleInfo.DisplayName = TEXT("Flecs Rewind Debugger");
	}

	void FTraceModule::OnAnalysisBegin(TraceServices::IAnalysisSession& InSession)
	{
		const TSharedPtr<FProvider> Provider = MakeShared<FProvider>(InSession);
		InSession.AddProvider(FProvider::ProviderName, Provider, Provider);
		InSession.AddAnalyzer(new FAnalyzer(InSession, *Provider));
	}

	void FTraceModule::GetLoggers(TArray<const TCHAR*>& OutLoggers)
	{
		OutLoggers.Add(TEXT("Flecs"));
	}

	const TCHAR* FTraceModule::GetCommandLineArgument()
	{
		return TEXT("flecsrewinddebugger");
	}
} // namespace UE::Flecs::RewindDebugger
