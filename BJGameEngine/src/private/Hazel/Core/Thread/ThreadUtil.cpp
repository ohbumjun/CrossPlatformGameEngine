#include "Hazel/Core/Thread/ThreadUtil.h"
#include "hzpch.h"

const DWORD MS_VC_EXCEPTION = 0x406D1388;
#pragma pack(push, 8)
typedef struct tagTHREADNAME_INFO
{
    DWORD dwType;     // Must be 0x1000.
    LPCSTR szName;    // Pointer to name (in user addr space).
    DWORD dwThreadID; // Thread ID (-1=caller thread).
    DWORD dwFlags;    // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

static void get_last_error_message(const wchar_t *buf, size_t size)
{
    //wchar_t buf[256];
    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL,
                   GetLastError(),
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                   (wchar_t *)buf,
                   (DWORD)size,
                   NULL);

    wchar_t *ptr = (wchar_t *)buf;
    while (*ptr)
    {
        if (*ptr == '\r')
        {
            *ptr = 0x0000;
            break;
        }
        ptr++;
    }
}

namespace Hazel
{
Atomic::Atomic(const Atomic &o){

};

int Atomic::GetValue()
{
    return value;
};

void ThreadUtils::InitThread(ThreadInfo *thread,
                             void *(*thread_function)(void *),
                             void *arg)
{
}

void ThreadUtils::RunThread(ThreadInfo *thread,
                            void *(*thread_function)(void *),
                            void *arg)
{
    // 실제 thread 를 만들어서 os 로부터 handle 값을 리턴받는다.
    thread->handle = (HANDLE)_beginthreadex(
        NULL, // default security attributes
        0,    // use default stack Size
        reinterpret_cast<unsigned(__stdcall *)(void *)>(
            thread_function),         // Cast to correct signature
        arg,                          // argument to thread function
        0,                            // use default creation flags
        (unsigned int *)&thread->id); // returns the thread identifier

    // 위의 함수는 새로운 thread 를 만드는 역할을 한다.
    // 바로 thread 가 call back function 을 실행하도록 하지는 않는다. 쓰레드가 일을 진행하게 할지 말지는
    // 운영체제 (더 구체적으로는 스케줄러)의 역할이기 때문이다.

    if (thread->handle == INVALID_HANDLE_VALUE)
    {
        wchar_t buf[256];
        get_last_error_message(buf, sizeof(buf));

        THROW("lv_thread_run is failed %ls", buf);
    }
    else
    {
        // https://docs.microsoft.com/ko-kr/previous-versions/visualstudio/visual-studio-2015/debugger/how-to-set-a-thread-name-in-native-code?view=vs-2015&redirectedfrom=MSDN
        THREADNAME_INFO info;
        info.dwType = 0x1000;
        info.szName = thread->name;
        info.dwThreadID = thread->id;
        info.dwFlags = 0;
#pragma warning(push)
#pragma warning(disable : 6320 6322)
        __try
        {
            RaiseException(MS_VC_EXCEPTION,
                           0,
                           sizeof(info) / sizeof(ULONG_PTR),
                           (ULONG_PTR *)&info);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
        }
#pragma warning(pop)
    }
}

bool ThreadUtils::JoinThread(ThreadInfo *thread)
{
    /*
	 "joining" a thread refers to the process of waiting for a specific thread to finish its execution
	 before continuing with the execution
	*/
    if (thread == NULL)
    {
        THROW("LvThread is null");
        return false;
    }

    if (thread->handle == NULL)
    {
        THROW("LvThread is null");
        return false;
    }

    if (thread->handle != NULL)
    {
        /*
		- 해당 쓰레드가 signaled 상태가 될 때까지 기다린다.
		  SleepConditionVariableCS  의 차이점은, SleepConditionVariableCS  함수는 특정 조건이 만족할 때까지 wait 하게 하는 것이라면
		  WaitForSingleObjectEx 는 보다 general 한 목적으로, 특정 조건에 한정된 것은 아니라는 점에서 차이가 있다.
		
        - 특정 Thread 를 생성해서 할일을 할당할 때는 해당 Thread 가 Non-Signaled 상태이다. 
          이후 해당 쓰레드가 할일을 끝내면, 해당 쓰레드는 Signaled 상태가 된다.

        - 즉, 아래 함수는 결과적으로 해당 쓰레드가 할일을 끝내고 Signaled 상태가 될 때까지 기다린다.
        */
        WaitForSingleObjectEx(thread->handle, INFINITE, FALSE);
    }
    return true;
}

void ThreadUtils::ExitThread(ThreadInfo *thread, void *exitCode)
{
    if (thread == NULL)
    {
        THROW("LvThread is null");
    }

    if (thread->handle == NULL)
    {
        THROW("LvThread is null");
    }

    CloseHandle(thread->handle);

    if (GetExitCodeThread(thread->handle, (LPDWORD)exitCode))
    {
        wchar_t buf[256];
        get_last_error_message(buf, sizeof(buf));
        THROW("TerminateThread Error Code %ls", buf);
        //LV_THROW("TerminateThread Error Code %d", GetLastError());
    }

    thread->handle = NULL;
}

void ThreadUtils::KillThread(ThreadInfo *thread, void *exitCode)
{
    if (TerminateThread(thread->handle, (DWORD)exitCode) == false)
    {
        wchar_t buf[256];
        get_last_error_message(buf, sizeof(buf));
        THROW("TerminateThread Error Code %ls", buf);
    }
}

void ThreadUtils::SetPriorityOfThread(ThreadInfo *thread, int priority)
{
    // In Windows, thread priorities range from THREAD_PRIORITY_IDLE(lowest) to THREAD_PRIORITY_TIME_CRITICAL(highest).
    // SetThreadPriority(thread->handle, priority);
    SetThreadPriority(thread, priority);
}

void ThreadUtils::SetThreadAffinity(ThreadInfo *thread, int affinity)
{
    if (thread == NULL)
    {
        THROW("LvThread is null");
    }

    if (thread->handle == NULL)
    {
        THROW("LvThread is null");
    }

    SetThreadAffinityMask(thread->handle, affinity);
}

int ThreadUtils::GetCurrentThreadName(char *name)
{
    if (ThreadUtils::IsCurrentMainThread())
    {
        strcpy_s(name, strlen("Main Thread") + 1, "Main Thread");
        return 0;
    }
    else
    {
        wchar_t *threadName = NULL;

        HRESULT hr = GetThreadDescription(GetCurrentThread(), &threadName);

        if (SUCCEEDED(hr))
        {
            // wchar 의 utf-8 버전 문자열 크기를 리턴한다.
            int size = WideCharToMultiByte(CP_UTF8,
                                           WC_NO_BEST_FIT_CHARS,
                                           threadName,
                                           -1,
                                           NULL,
                                           NULL,
                                           NULL,
                                           NULL);

            // wchar type 의 threadName 문자열을, utf-8 type의 name 으로 변환해준다.
            WideCharToMultiByte(CP_UTF8,
                                WC_NO_BEST_FIT_CHARS,
                                threadName,
                                -1,
                                name,
                                size,
                                NULL,
                                NULL);

            // threadName 에 할당된 메모리를 해제해준다.
            LocalFree(threadName);
        }

        return hr;
    }
}

int ThreadUtils::SetCurrentThreadName(ThreadInfo *thread, const char *name)
{
    return SetThreadDescription(thread->handle, (PCWSTR)name);
}

void ThreadUtils::SleepThread(unsigned long milliseconds)
{
    Sleep(milliseconds);
}

bool ThreadUtils::YieldThread()
{
    return SwitchToThread();
}

size_t ThreadUtils::GetCurrentStackLimit()
{
    // ULONG_PTR : unsigned long 형태로, 메모리 주소를 저장하는 변수 type 으로 많이 쓰인다.
    ULONG_PTR stackBase = 0;

    ULONG_PTR stackLimit = 0;

    GetCurrentThreadStackLimits(&stackLimit, &stackBase);

    // 현재 thread 가 가지고 있는 stack 의 size 를 리턴해준다.
    // stackBase	: top of stack		(highest memory address in stack)
    // stackLimit   : bottom of stack	(lowest memory address in stack)
    // 참고 : stack 은 high -> low 방향으로 메모리를 할당해간다.
    return (size_t)(stackBase - stackLimit);
}

unsigned long ThreadUtils::GetCurrentThreadID(void)
{
    return ((unsigned long)GetCurrentThreadId());
}

bool ThreadUtils::IsCurrentMainThread()
{
    return ThreadUtils::GetMainThreadID() == ThreadUtils::GetCurrentThreadID();
}

/*
메인 쓰레드 내에서 1회 호출하여 초기화할 필요가 있다.
*/
unsigned long ThreadUtils::GetMainThreadID()
{
    static unsigned long mainThreadId = GetCurrentThreadID();

    return mainThreadId;
}

unsigned int ThreadUtils::GetThreadHardwareCount()
{
    return std::thread::hardware_concurrency();
}

CRIC_SECT *ThreadUtils::CreateCritSect()
{
    CRIC_SECT *sect = (CRIC_SECT *)malloc(sizeof(CRIC_SECT));

    InitializeCriticalSection(&sect->handle);

    sect->isInit = true;

    return sect;
};

void ThreadUtils::LockCritSect(CRIC_SECT *sect)
{
    if (sect->isInit == false)
    {
        InitializeCriticalSection(&sect->handle);
        sect->isInit = true;
    }

    EnterCriticalSection(&sect->handle);
}

bool ThreadUtils::TryLockCritSect(CRIC_SECT *sect)
{
    if (sect == nullptr)
    {
        THROW("sect is null");
    }

    // Mutex
    //return WaitForSingleObject(mutex->handle, 0) == WAIT_OBJECT_0;

    return TryEnterCriticalSection(&sect->handle) != 0;
}

void ThreadUtils::UnlockCritSect(CRIC_SECT *sect)
{
    LeaveCriticalSection(&sect->handle);
}

void ThreadUtils::DestroyCritSect(CRIC_SECT *sect)
{
    DeleteCriticalSection(&sect->handle);
    free(sect);
}

void ThreadUtils::InitSpinLock(SpinLock *spinlock)
{
    // 해당 spinlock 의 atomic value 를 0 으로 초기화
    SetAtomic(&spinlock->flag, 0);
}

bool ThreadUtils::TryLockSpinLock(SpinLock *spinlock)
{
    // InitSpinLock 시에 '0'으로 설정
    // old : 0
    // new : 1

    // 만약 spinlock 값이 '0'이었다면, '1' 로 update 되고 true 리턴
    //					  '1'이었다면, '1' 로 update 되지 않고 false 를 리턴
    return !CompareAndSwapAtomic(&spinlock->flag, 0, 1);
}

void ThreadUtils::LockSpinLock(SpinLock *spinlock)
{
    // TryLockSpinLock 값이 false 를 리턴할 때까지 무한 반복
    // 즉, spin_lock 값이 '1' 일때까지 무한 반복
    //     다른 쓰레드에서 해당 값을 '1' 로 설정할 때까지 무한 대기
    while (TryLockSpinLock(spinlock))
        ;
}

void ThreadUtils::UnlockSpinLock(SpinLock *spinlock)
{
    // InitSpinLock 시에 '0'으로 설정
    // old : 1
    // new : 0

    // 만약 spinlock 값이 '1'이었다면, '0' 로 update 되고 true 리턴
    //					  '0'이었다면, '0' 로 update 되지 않고 false 를 리턴
    CompareAndSwapAtomic(&spinlock->flag, 1, 0);
}

int ThreadUtils::SetAtomic(Atomic *a, int val)
{
    // _InterlockedExchange : atomic exchange 연산을 수행하는 함수
    return _InterlockedExchange((long *)&a->value, val);
}

int ThreadUtils::GetAtomic(Atomic *a)
{
    int r;

    do
    {
        r = a->value;

        // CompareAndSwapAtomic(a, r, r)
        // false ? : a->value 가 r 과 다르다는 의미
        // true  ? : a->value 가 r 과 같다는 의미
        // 여러 쓰레드에서 해당 atomic value 에 동시 접근한다고 생각해보자.
        // a->value 가 1이었다.

        // A Thread : r = a->value; 까지만, 실행, CompareAndSwapAtomic 을 실행하기 전. r 은 1
        // B Thread : 그 사이에 atomic add 를 함. a->value 는 2;

        // 자. A Thread 는 CompareAndSwapAtomic 을 통해서 현재 atomic value 를 '1' 로 update 하고자 한다.
        // A Thread : CompareAndSwapAtomic(a(2), r(1), r(1)) => 2와 1 은 다르다. 따라서 return 값은 false
        // 다시 연산을 시도한다

        // r = a->value (2)
        // A Thread : atomic->value 를 '2' 로 update 시도한다.
        // CompareAndSwapAtomic(a(2), r(2), r(2)) => 2와 2 은 같다. 따라서 return true
    } while (!CompareAndSwapAtomic(a, r, r));

    return r;
}

int ThreadUtils::AddAtomic(Atomic *a, int v)
{
    /*
	_InterlockedExchangeAdd 와 _InterlockedIncrement 의 차이점

	1) _InterlockedExchangeAdd
	- 기능 : 특정값만큼 증가 (꼭 1이 아니어도 된다)
	- 리턴 : add 전의 origin value

	2) _InterlockedIncrement
	- 기능 : 1 만 증가
	- 리턴 : add 전 origin value
	*/
    return _InterlockedExchangeAdd((long *)&a->value, v);
}

int ThreadUtils::IncreaseAtomic(Atomic *a)
{
    return _InterlockedIncrement((long *)&a->value);
}

int ThreadUtils::DecreaseAtomic(Atomic *a)
{
    return _InterlockedDecrement((long *)&a->value);
}

bool ThreadUtils::CompareAndSwapAtomic(Atomic *atomic, int oldVal, int newVal)
{
    // 기능    : 특정 값 ~ atomic 값을 비교한다. 만약 동일하다면 새로운 value 로 replace 한다.
    // 리턴 값 : compare and swap 연산 이전 값
    // newValue : replace 하고자 하는 값
    // oldValue : 현재 atomic->value 와 비교하고자 하는 값.

    // True  ? : newValu 로 update 성공
    // False ? : update X
    return _InterlockedCompareExchange((long *)&atomic->value,
                                       (long)newVal,
                                       (long)oldVal) == (long)oldVal;
}

ConditionVar *ThreadUtils::CreateCondition()
{
    ConditionVar *r = (ConditionVar *)malloc(sizeof(ConditionVar));

    InitializeConditionVariable(&r->handle);

    return r;
}

void ThreadUtils::DestroyCondition(ConditionVar *condition)
{
    free(condition);
}

bool ThreadUtils::WaitCondition(ConditionVar *con,
                                CRIC_SECT *mutex,
                                uint32 time)
{
    /*
	멀티쓰레드 환경에서, 다수의 쓰레드가 특정 condition 이 true 가 될때까지 기다리게 할 때 사용되는 함수이다.

	- 쓰레드는 condition variable 이 true 가 될 때까지 기다리고(블로킹), 그 동안 critical section 을 release 한다. (unlock)
	- 다른 running thread 가 WakeConditionVariable 함수를 호출하면, 해당 condition variable 에 대한 대기큐에 들어가 있는
	  블로킹 쓰레드를 깨운다.
	- 그리고 다시 해당 condition variable 이 true 인지 아닌지를 검사한다.
	*/
    return SleepConditionVariableCS(&con->handle, &mutex->handle, time);
}

void ThreadUtils::NotifyOneCondtion(ConditionVar *con)
{
    WakeConditionVariable(&con->handle);
}

void ThreadUtils::NotifyAllCondtion(ConditionVar *con)
{
    WakeAllConditionVariable(&con->handle);
}

Semaphore *ThreadUtils::CreateCustomSemaphore(int init_count)
{
    Semaphore *r;
    r = (Semaphore *)malloc(sizeof(Semaphore));
    // init_count : 세마포 개체의 초기 개수입니다.세마포의 상태는 개수가 0보다 크고 0일 때 부호가 없는 경우 신호를 보냅니다.
    //				대기 함수가 세마포를 기다리고 있던 스레드를 해제할 때마다 수가 1씩 감소합니다
    // lMaximumCount(32 * 1024) : 세마포 개체 최대 개수
    // NULL : name
    r->handle = CreateSemaphore(NULL, init_count, 32 * 1024, NULL);
    r->count = init_count;

    if (r->handle == NULL)
    {
        THROW("Couldn't create semaphore");
    }

    return r;
}

void ThreadUtils::DestroyCustomSemaphore(Semaphore *sema)
{
    if (sema != NULL)
    {
        if (sema->handle != NULL)
        {
            // semaphore 와 관련된 리소스들을 clear 해준다.
            CloseHandle(sema->handle);
            sema->handle = 0;
        }
        free(sema);
    }
}

int ThreadUtils::SignalCustomSemaphore(Semaphore *sema)
{
    if (sema == NULL)
    {
        THROW("Semaphore is null");
    }

    // InterlockedIncrement , InterlockedDecrement
    // - 멀티쓰레드 환경에서 특정 변수 값을 1증가, 1 감소 시켜주는 함수

    // InterlockedIncrement(&sema->count);
    // InterlockedIncrement((LONG volatile*)&sema->count);
    InterlockedIncrement((LONG *)&sema->count);

    // release one unit from semaphore
    if (ReleaseSemaphore(sema->handle, 1, NULL) == FALSE)
    {
        // InterlockedDecrement((LONG volatile*)&sema->count);
        InterlockedDecrement((LONG *)&sema->count);
        THROW("ReleaseSemaphore() failed");
    }
    return 0;
}

int ThreadUtils::WaitSemaphore(Semaphore *sema, int timeout)
{
    if (sema == NULL)
    {
        THROW("Semaphore is null");
    }

    int r;
    DWORD millisec;


    if (timeout == THREAD_WAIT_INFINITE)
    {
        millisec = INFINITE;
    }
    else
    {
        millisec = (DWORD)timeout;
    }

    // millisec 동안 세마포어 객체에 대해 wait 을 한다.
    // 결과
    // 1) semaphore 에 대해 signal 을 하거나 (..? release 하거나가 아닌가?)
    // 2) timeout
    switch (WaitForSingleObjectEx(sema->handle, millisec, FALSE))
    {
        // 해당 milisec 시간 안에 성공적으로 signaled 되었다는 의미
    case WAIT_OBJECT_0:
        InterlockedDecrement((LONG *)&sema->count);
        r = 0;
        break;
    case WAIT_TIMEOUT:
        r = THREAD_WAIT_TIMEDOUT;
        break;
    default:
        THROW("WaitForSingleObject() failed");
        break;
    }

    return r;
}

SpinLock::SpinLock()
{
    Init();
}

void SpinLock::Init()
{
    _InterlockedExchange((long *)&flag.value, 0);
}

bool SpinLock::TryLock()
{
    // 기능    : 특정 값 ~ atomic 값을 비교한다. 만약 동일하다면 새로운 value 로 replace 한다.
    // 리턴 값 : compare and swap 연산 이전 값
    // newValue : replace 하고자 하는 값
    // oldValue : 현재 atomic->value 와 비교하고자 하는 값.

    // True  ? : newValu 로 update 성공
    // False ? : update X

    return !_InterlockedCompareExchange((long *)&flag.value,
                                        (long)0,
                                        (long)1) == (long)1;
    // return !CompareAndSwapAtomic(&flag, 0, 1);
}

void SpinLock::Lock()
{
    // TryLockSpinLock 값이 false 를 리턴할 때까지 무한 반복
    // 즉, spin_lock 값이 '1' 일때까지 무한 반복
    //     다른 쓰레드에서 해당 값을 '1' 로 설정할 때까지 무한 대기
    while (TryLock())
        ;
}

void SpinLock::Unlock()
{
    // InitSpinLock 시에 '0'으로 설정
    // old : 1
    // new : 0

    // 만약 spinlock 값이 '1'이었다면, '0' 로 update 되고 true 리턴
    //					  '0'이었다면, '0' 로 update 되지 않고 false 를 리턴
    // CompareAndSwapAtomic(&flag.value, 1, 0);
    _InterlockedCompareExchange((long *)&flag.value, (long)1, (long)0) ==
        (long)0;
}

} // namespace Hazel