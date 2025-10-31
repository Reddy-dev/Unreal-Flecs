// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include "flecs.h"

namespace flecs
{
	struct FFlecsRAIIPerfTraceWindow
	{
		FFlecsRAIIPerfTraceWindow(const char* InName, const char *file, size_t line)
			: Name(InName)
			, File(file)
			, Line(line)
		{
			ecs_os_perf_trace_push_(file, line, Name.c_str());
		}

		~FFlecsRAIIPerfTraceWindow()
		{
			ecs_os_perf_trace_pop_(Name.c_str(), Line, File.c_str());
		}

		flecs::string Name;
		flecs::string File;
		size_t Line;
	}; // struct FFlecsRAIIPerfTraceWindow

	#define FLECS_PERF_TRACE_WINDOW_SCOPE(name) \
		flecs::FFlecsRAIIPerfTraceWindow RAII_PerfTraceWindow##__LINE__(name, __FILE__, __LINE__);
	
} // namespace flecs

