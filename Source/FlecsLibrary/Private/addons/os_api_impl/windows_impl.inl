/**
 * @file addons/os_api_impl/posix_impl.inl
 * @brief Builtin Windows implementation for OS API.
 */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <windows.h>

typedef struct ecs_win_thread_t {
    HANDLE thread;
    ecs_os_thread_callback_t callback;
    void *arg;
} ecs_win_thread_t;

static
DWORD flecs_win_thread(void *ptr) {
    const ecs_win_thread_t *thread = ptr;
    thread->callback(thread->arg);
    return 0;
}

static
ecs_os_thread_t win_thread_new(
    ecs_os_thread_callback_t callback, 
    void *arg)
{
    ecs_win_thread_t *thread = ecs_os_malloc_t(ecs_win_thread_t);
    thread->arg= arg;
    thread->callback = callback;
    thread->thread = CreateThread(
        NULL, 0, (LPTHREAD_START_ROUTINE)flecs_win_thread, thread, 0, NULL);
    return (ecs_os_thread_t)(uintptr_t)thread;
}

static
void* win_thread_join(
    ecs_os_thread_t thr)
{
    ecs_win_thread_t *thread = (ecs_win_thread_t*)(uintptr_t)thr;
    const DWORD r = WaitForSingleObject(thread->thread, INFINITE);
    if (r == WAIT_FAILED) {
        ecs_err("win_thread_join: WaitForSingleObject failed");
    }
    ecs_os_free(thread);
    return NULL;
}

static
ecs_os_thread_id_t win_thread_self(void)
{
    return (ecs_os_thread_id_t)GetCurrentThreadId();
}

static
int32_t win_ainc(
    int32_t *count) 
{
    return InterlockedIncrement((volatile long*)count);
}

static
int32_t win_adec(
    int32_t *count) 
{
    return InterlockedDecrement((volatile long*)count);
}

static
int64_t win_lainc(
    int64_t *count) 
{
    return InterlockedIncrement64(count);
}

static
int64_t win_ladec(
    int64_t *count) 
{
    return InterlockedDecrement64(count);
}

static
ecs_os_mutex_t win_mutex_new(void) {
    CRITICAL_SECTION *mutex = ecs_os_malloc_t(CRITICAL_SECTION);
    InitializeCriticalSection(mutex);
    return (ecs_os_mutex_t)(uintptr_t)mutex;
}

static
void win_mutex_free(
    ecs_os_mutex_t m) 
{
    CRITICAL_SECTION *mutex = (CRITICAL_SECTION*)(intptr_t)m;
    DeleteCriticalSection(mutex);
    ecs_os_free(mutex);
}

static
void win_mutex_lock(
    ecs_os_mutex_t m) 
{
    CRITICAL_SECTION *mutex = (CRITICAL_SECTION*)(intptr_t)m;
    EnterCriticalSection(mutex);
}

static
void win_mutex_unlock(
    ecs_os_mutex_t m) 
{
    CRITICAL_SECTION *mutex = (CRITICAL_SECTION*)(intptr_t)m;
    LeaveCriticalSection(mutex);
}

static
ecs_os_cond_t win_cond_new(void) {
    CONDITION_VARIABLE *cond = ecs_os_malloc_t(CONDITION_VARIABLE);
    InitializeConditionVariable(cond);
    return (ecs_os_cond_t)(uintptr_t)cond;
}

static 
void win_cond_free(
    ecs_os_cond_t c) 
{
    ecs_os_free((CONDITION_VARIABLE*)(uintptr_t)c);
}

static 
void win_cond_signal(
    ecs_os_cond_t c) 
{
    CONDITION_VARIABLE *cond = (CONDITION_VARIABLE*)(intptr_t)c;
    WakeConditionVariable(cond);
}

static 
void win_cond_broadcast(
    ecs_os_cond_t c) 
{
    CONDITION_VARIABLE *cond = (CONDITION_VARIABLE*)(intptr_t)c;
    WakeAllConditionVariable(cond);
}

static 
void win_cond_wait(
    ecs_os_cond_t c, 
    ecs_os_mutex_t m) 
{
    CRITICAL_SECTION *mutex = (CRITICAL_SECTION*)(intptr_t)m;
    CONDITION_VARIABLE *cond = (CONDITION_VARIABLE*)(intptr_t)c;
    SleepConditionVariableCS(cond, mutex, INFINITE);
}

static bool win_time_initialized;
static double win_time_freq;
static LARGE_INTEGER win_time_start;
static ULONG win_current_resolution;

static
void win_time_setup(void) {
    if ( win_time_initialized) {
        return;
    }
    
    win_time_initialized = true;

    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&win_time_start);
    win_time_freq = (double)freq.QuadPart / 1000000000.0;
}

static
void win_sleep(
    int32_t sec, 
    int32_t nanosec) 
{
    LARGE_INTEGER ft;

    ft.QuadPart = -((int64_t)sec * 10000000 + (int64_t)nanosec / 100);

    const HANDLE timer = CreateWaitableTimer(NULL, TRUE, NULL);
    SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
    WaitForSingleObject(timer, INFINITE);
    CloseHandle(timer);
}

static
void win_enable_high_timer_resolution(bool enable)
{
    const HMODULE hntdll = GetModuleHandle(TEXT("ntdll.dll"));
    if (!hntdll) {
        return;
    }

    union {
        LONG (__stdcall *f)(
            ULONG desired, BOOLEAN set, ULONG * current);
        FARPROC p;
    } func;

    func.p = GetProcAddress(hntdll, "NtSetTimerResolution");
    if(!func.p) {
        return;
    }

    ULONG current, resolution = 10000; /* 1 ms */

    if (!enable && win_current_resolution) {
        func.f(win_current_resolution, 0, &current);
        win_current_resolution = 0;
        return;
    } else if (!enable) {
        return;
    }

    if (resolution == win_current_resolution) {
        return;
    }

    if (win_current_resolution) {
        func.f(win_current_resolution, 0, &current);
    }

    if (func.f(resolution, 1, &current)) {
        /* Try setting a lower resolution */
        resolution *= 2;
        if(func.f(resolution, 1, &current)) return;
    }

    win_current_resolution = resolution;
}

static
uint64_t win_time_now(void) {
    LARGE_INTEGER qpc_t;
    QueryPerformanceCounter(&qpc_t);
    const uint64_t now = (uint64_t)((double)qpc_t.QuadPart / win_time_freq);

    return now;
}

static
void win_fini(void) {
    if (ecs_os_api.flags_ & EcsOsApiHighResolutionTimer) {
        win_enable_high_timer_resolution(false);
    }
}

void ecs_set_os_api_impl(void) {
    ecs_os_set_api_defaults();

    ecs_os_api_t api = ecs_os_api;

    api.thread_new_ = win_thread_new;
    api.thread_join_ = win_thread_join;
    api.thread_self_ = win_thread_self;
    api.task_new_ = win_thread_new;
    api.task_join_ = win_thread_join;
    api.ainc_ = win_ainc;
    api.adec_ = win_adec;
    api.lainc_ = win_lainc;
    api.ladec_ = win_ladec;
    api.mutex_new_ = win_mutex_new;
    api.mutex_free_ = win_mutex_free;
    api.mutex_lock_ = win_mutex_lock;
    api.mutex_unlock_ = win_mutex_unlock;
    api.cond_new_ = win_cond_new;
    api.cond_free_ = win_cond_free;
    api.cond_signal_ = win_cond_signal;
    api.cond_broadcast_ = win_cond_broadcast;
    api.cond_wait_ = win_cond_wait;
    api.sleep_ = win_sleep;
    api.now_ = win_time_now;
    api.fini_ = win_fini;

    win_time_setup();

    if (ecs_os_api.flags_ & EcsOsApiHighResolutionTimer) {
        win_enable_high_timer_resolution(true);
    }

    ecs_os_set_api(&api);
}
