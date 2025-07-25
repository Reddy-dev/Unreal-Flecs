﻿// Elie Wiese-Namir © 2025. All Rights Reserved.

#pragma once

#include <condition_variable>

#include "CoreMinimal.h"
#include "Experimental/Async/ConditionVariable.h"
#include "flecs/os_api.h"
#include "Logs/FlecsCategories.h"
#include "SolidMacros/Macros.h"

DECLARE_STATS_GROUP(TEXT("FlecsOS"), STATGROUP_FlecsOS, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("FlecsOS::TaskThread"), STAT_FlecsOS, STATGROUP_FlecsOS);

class FFlecsRunnable final : public FRunnable
{
public:
	FORCEINLINE FFlecsRunnable(ecs_os_thread_callback_t InCallback, void* InData)
		: Callback(InCallback)
		, Data(InData)
		, bStopped(false)
	{
	}

	FORCEINLINE virtual uint32 Run() override
	{
		while (!bStopped.load())
		{
			Callback(Data);
			break;
		}
		
		return 0;
	}

	FORCEINLINE virtual void Stop() override
	{
		bStopped.store(true);
	}

private:
	ecs_os_thread_callback_t Callback;
	void* Data;
	std::atomic<bool> bStopped;
}; // class FFlecsRunnable

struct FFlecsThreadWrapper
{
	static constexpr EThreadPriority TaskThread = TPri_Highest;
	
	FFlecsRunnable* Runnable = nullptr;
	FRunnableThread* RunnableThread = nullptr;
	std::atomic<bool> bJoined { false };

	FORCEINLINE FFlecsThreadWrapper(ecs_os_thread_callback_t Callback, void* Data)
	{
		Runnable = new FFlecsRunnable(Callback, Data);
		RunnableThread = FRunnableThread::Create(
			Runnable, TEXT("FlecsThreadWrapper"), 0, TaskThread);
	}

	FORCEINLINE ~FFlecsThreadWrapper()
	{
		if (!bJoined.load() && RunnableThread)
		{
			Stop();
			Join();

			delete RunnableThread;
			RunnableThread = nullptr;
		}
	}

	FORCEINLINE void Stop() const
	{
		if (Runnable)
		{
			Runnable->Stop();
		}
	}

	FORCEINLINE void Join()
	{
		if (!bJoined.exchange(true))
		{
			if (RunnableThread)
			{
				RunnableThread->WaitForCompletion();
				delete RunnableThread;
				RunnableThread = nullptr;
			}
			
			if (Runnable)
			{
				delete Runnable;
				Runnable = nullptr;
			}
		}
	}
	
}; // struct FFlecsThreadWrapper

struct FFlecsTask
{
	static constexpr ENamedThreads::Type TaskThread = ENamedThreads::AnyHiPriThreadNormalTask;
	
	FGraphEventRef TaskEvent;

	FORCEINLINE FFlecsTask(const ecs_os_thread_callback_t Callback, void* Data)
	{
		TaskEvent = FFunctionGraphTask::CreateAndDispatchWhenReady(
			[Callback, Data]()
			{
				Callback(Data);
			},
			GET_STATID(STAT_FlecsOS), nullptr, TaskThread);
	}

	FORCEINLINE ~FFlecsTask()
	{
		if (TaskEvent.IsValid())
		{
			TaskEvent->Wait();
		}
	}

	FORCEINLINE void Wait() const
	{
		if (TaskEvent.IsValid())
		{
			FTaskGraphInterface::Get().WaitUntilTaskCompletes(TaskEvent);
		}
	}
	
}; // struct FFlecsTask

struct FFlecsConditionWrapper
{
	UE::FConditionVariable ConditionalVariable;
	FCriticalSection* Mutex;
}; // struct ConditionWrapper

struct FOSApiInitializer
{
	static constexpr uint32 FlecsMemoryDefaultAlignment = 64;
	
	FOSApiInitializer()
	{
		UE_LOG(LogFlecsCore, Log, TEXT("Initializing Flecs OS API"));
		InitializeOSAPI();
	}
	
	void InitializeOSAPI()
	{
        ecs_os_set_api_defaults();
		
        ecs_os_api_t os_api = ecs_os_api;

	    os_api.mutex_new_ = []() -> ecs_os_mutex_t
	    {
	    	FCriticalSection* Mutex = new FCriticalSection();
	        return reinterpret_cast<ecs_os_mutex_t>(Mutex);
	    };

		os_api.mutex_free_ = [](ecs_os_mutex_t Mutex)
		{
			FCriticalSection* MutexPtr = reinterpret_cast<FCriticalSection*>(Mutex);
			delete MutexPtr;
		};

		os_api.mutex_lock_ = [](ecs_os_mutex_t Mutex)
		{
			solid_checkf(Mutex, TEXT("Mutex is nullptr"));
			FCriticalSection* MutexPtr = reinterpret_cast<FCriticalSection*>(Mutex);
			MutexPtr->Lock();
		};

		os_api.mutex_unlock_ = [](ecs_os_mutex_t Mutex)
		{
			FCriticalSection* MutexPtr = reinterpret_cast<FCriticalSection*>(Mutex);
			MutexPtr->Unlock();
		};

		os_api.cond_new_ = []() -> ecs_os_cond_t
		{
			FFlecsConditionWrapper* Wrapper = new FFlecsConditionWrapper();
			Wrapper->Mutex = new FCriticalSection();
			return reinterpret_cast<ecs_os_cond_t>(Wrapper);
		};

		os_api.cond_free_ = [](ecs_os_cond_t Cond)
		{
			if LIKELY_IF(Cond)
			{
				FFlecsConditionWrapper* Wrapper = reinterpret_cast<FFlecsConditionWrapper*>(Cond);
				delete Wrapper->Mutex;
				Wrapper->Mutex = nullptr;
				delete Wrapper;
			}
		};

		os_api.cond_signal_ = [](ecs_os_cond_t Cond)
		{
			if LIKELY_IF(Cond)
			{
				FFlecsConditionWrapper* Wrapper = reinterpret_cast<FFlecsConditionWrapper*>(Cond);
				Wrapper->ConditionalVariable.NotifyOne();
			}
		};

		os_api.cond_broadcast_ = [](ecs_os_cond_t Cond)
		{
			if LIKELY_IF(Cond)
			{
				FFlecsConditionWrapper* Wrapper = reinterpret_cast<FFlecsConditionWrapper*>(Cond);
				Wrapper->ConditionalVariable.NotifyAll();
			}
		};

		os_api.cond_wait_ = [](ecs_os_cond_t Cond, ecs_os_mutex_t Mutex)
		{
			if LIKELY_IF(Cond && Mutex)
			{
				FFlecsConditionWrapper* Wrapper = reinterpret_cast<FFlecsConditionWrapper*>(Cond);
				FCriticalSection* CritSection = reinterpret_cast<FCriticalSection*>(Mutex);

				Wrapper->ConditionalVariable.Wait(*CritSection);
			}
		};

		os_api.thread_new_ = [](ecs_os_thread_callback_t Callback, void* Data) -> ecs_os_thread_t
		{
			FFlecsThreadWrapper* ThreadWrapper = new FFlecsThreadWrapper(Callback, Data);
			return reinterpret_cast<ecs_os_thread_t>(ThreadWrapper);
		};

		os_api.thread_join_ = [](ecs_os_thread_t Thread) -> void*
		{
			if LIKELY_IF(FFlecsThreadWrapper* ThreadWrapper = reinterpret_cast<FFlecsThreadWrapper*>(Thread))
			{
				ThreadWrapper->Join();
				delete ThreadWrapper;
			}
			
			return nullptr;
		};

		os_api.thread_self_ = []() -> ecs_os_thread_id_t
		{
			return FPlatformTLS::GetCurrentThreadId();
		};

		os_api.task_new_ = [](ecs_os_thread_callback_t Callback, void* Data) -> ecs_os_thread_t
		{
			FFlecsTask* FlecsTask = new FFlecsTask(Callback, Data);
			return reinterpret_cast<ecs_os_thread_t>(FlecsTask);
		};

		os_api.task_join_ = [](ecs_os_thread_t Thread) -> void*
		{
			FFlecsTask* FlecsTask = reinterpret_cast<FFlecsTask*>(Thread);
			solid_check(FlecsTask);
			
			FlecsTask->Wait();
			delete FlecsTask;
			
			return nullptr;
		};
		
        os_api.sleep_ = [](int32_t Seconds, int32_t Nanoseconds)
        {
            const double TotalSeconds = Seconds + Nanoseconds / 1e9;
            FPlatformProcess::SleepNoStats(static_cast<float>(TotalSeconds));
        };
		
        os_api.now_ = []() -> uint64_t
        {
            return FPlatformTime::Cycles64();
        };

        os_api.get_time_ = [](ecs_time_t* TimeOut)
        {
        	solid_check(TimeOut);
        	
            const double Seconds = FPlatformTime::Seconds();
            TimeOut->sec = static_cast<uint32_t>(Seconds);
        	
        	// Seconds to Nanoseconds: 1 second = 1e9 nanoseconds
            TimeOut->nanosec = static_cast<uint32_t>((Seconds - TimeOut->sec) * 1e9);
        };

        os_api.abort_ = []()
        {
        	#if !NO_LOGGING
        		UE_LOG(LogFlecsCore, Fatal, TEXT("Flecs - Aborting..."));
			#endif // SOLID_LOG_ENABLED
        	
        	FGenericPlatformMisc::RequestExit(false);
        };

        os_api.log_ = [](int32_t Level, const char* File, int32_t Line, const char* Message)
        {
#if !NO_LOGGING
        	const TCHAR* LogFile = StringCast<TCHAR>(File).Get();
        	const TCHAR* LogMessage = StringCast<TCHAR>(Message).Get();
            switch (Level)
            {
                case -4: // Fatal
	                {
                		UE_LOG(LogFlecsCore, Fatal,
                			TEXT("Flecs - File: %s, Line: %d, Message: %s"),
							LogFile, Line, LogMessage);
	                }
                    break;
                case -3: // Error
	                {
                		UE_LOG(LogFlecsCore, Error,
                			TEXT("Error Flecs - File: %s, Line: %d, Message: %s"),
							LogFile, Line, LogMessage);
	                }
                    break;
                case -2: // Warning
	                {
                		UE_LOG(LogFlecsCore, Warning,
                			TEXT("Flecs - File: %s, Line: %d, Message: %s"),
							LogFile, Line, LogMessage);
	                }
                    break;
            	case 0: // Verbose
            		{
            			UE_LOG(LogFlecsCore, Verbose,
            				TEXT("Flecs - File: %s, Line: %d, Message: %s"),
							LogFile, Line, LogMessage);
            		}
                    break;
            	case 4: // Bookmark/Journal
            		{
            			TRACE_BOOKMARK(TEXT("Flecs - File: %s, Line: %d, Message: %s"),
            				LogFile, Line, LogMessage);
            			UE_LOG(LogFlecsJournal, VeryVerbose,
            				TEXT("Flecs - File: %s, Line: %d, Message: %s"),
            				LogFile, Line, LogMessage);
            		}
					break;
                default: // Info and Debug
            		{
            			UE_LOG(LogFlecsCore, Log,
            				TEXT("Flecs - File: %s, Line: %d, Message: %s"),
							LogFile, Line, LogMessage);
            		}
                    break;
            }
#endif // UNLOG_ENABLED
        };

		#ifdef FLECS_PERF_TRACE

		struct FFlecsProfilerTrace
		{
			FString FileName;
			uint32 Line;
			FString Name;

			FORCEINLINE FFlecsProfilerTrace(const FString& InFileName, const uint32 InLine, const FString& InName)
				: FileName(InFileName), Line(InLine), Name(InName)
			{
			}

			NO_DISCARD FORCEINLINE FString ToString() const
			{
				return FString::Printf(TEXT("FlecsProfilerTrace - File: %s, Line: %d, Name: %s"),
					*FileName, Line, *Name);
			}
			
		}; // struct FFlecsProfilerTrace

		thread_local TArray<FFlecsProfilerTrace> FlecsProfilerTraces;

		#endif // FLECS_PERF_TRACE

		os_api.perf_trace_push_ = [](const char* FileName, size_t Line, const char* Name)
		{
			#ifdef FLECS_PERF_TRACE
			
				solid_check(Line < std::numeric_limits<uint32>::max());
			
				FlecsProfilerTraces.Emplace(
					StringCast<TCHAR>(FileName).Get(), static_cast<uint32>(Line),
					StringCast<TCHAR>(Name).Get());
			
				FCpuProfilerTrace::OutputBeginDynamicEvent(Name, FileName, static_cast<uint32>(Line));
			
			#endif // FLECS_PERF_TRACE
		};

		os_api.perf_trace_pop_ = [](const char* FileName, size_t Line, const char* Name)
		{
			#ifdef FLECS_PERF_TRACE
			
				solid_check(Line < std::numeric_limits<uint32>::max());
			
				if (!FlecsProfilerTraces.IsEmpty())
				{
					const FFlecsProfilerTrace& Trace = FlecsProfilerTraces.Last();

					if (Trace.FileName != FileName || Trace.Name != Name)
					{
						UE_LOGFMT(LogFlecsCore, Error,
							"Flecs - Mismatched profiler trace pop: "
							"Got {TraceName} from {TraceFileName}:{TraceLine}, "
							"Expected {Name} from {FileName}:{Line}",
							*Trace.Name, *Trace.FileName, Trace.Line,
							Name, FileName, static_cast<uint32>(Line));
					}
					else
					{
						FlecsProfilerTraces.Pop();
					}
				}
				else
				{
					solid_checkf(false, TEXT("No matching Flecs profiler trace found for pop"));
				}
			
				FCpuProfilerTrace::OutputEndEvent();
			
			#endif // FLECS_PERF_TRACE
		};

		os_api.adec_ = [](int32_t* Value) -> int32
		{
			return FPlatformAtomics::InterlockedDecrement(Value);
		};

		os_api.ainc_ = [](int32_t* Value) -> int32
		{
			return FPlatformAtomics::InterlockedIncrement(Value);
		};

		os_api.lainc_ = [](int64_t* Value) -> int64
		{
			return FPlatformAtomics::InterlockedIncrement(Value);
		};

		os_api.ladec_ = [](int64_t* Value) -> int64
		{
			return FPlatformAtomics::InterlockedDecrement(Value);
		};

		os_api.malloc_ = [](int Size) -> void*
		{
			return FMemory::Malloc(Size, FlecsMemoryDefaultAlignment);
		};

		os_api.realloc_ = [](void* Ptr, int Size) -> void*
		{
			return FMemory::Realloc(Ptr, Size, FlecsMemoryDefaultAlignment);
		};

		os_api.calloc_ = [](int Size) -> void*
		{
			return FMemory::MallocZeroed(Size, FlecsMemoryDefaultAlignment);
		};

		os_api.free_ = [](void* Ptr)
		{
			FMemory::Free(Ptr);
		};
		
        ecs_os_set_api(&os_api);
	}
	
}; // struct FOSApiInitializer
