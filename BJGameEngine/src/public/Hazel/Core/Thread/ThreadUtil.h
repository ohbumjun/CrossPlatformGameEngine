#pragma once

#include <process.h>
#include <thread>


#define THREAD_WAIT_INFINITE (~(uint32)0)
#define THREAD_WAIT_TIMEDOUT 1
namespace Hazel
{
struct CRIC_SECT
{
    CRITICAL_SECTION handle;
    // 멀티쓰레드 환경에서 ,값이 쓰레드에 의해 변할 수 있으므로
    // 최적화 x
    volatile bool isInit;
};

struct Atomic
{
    Atomic() = default;

    Atomic(const Atomic &o);

    int GetValue();

    volatile int value;
};

typedef struct
{
    // #if defined(__WIN32__)
    CONDITION_VARIABLE handle;
    // #else
    // pthread_cond_t handle;
    // #endif

} ConditionVar;

typedef struct
{
    // ex. 초기값 == 5
    // 현재 몇개의 쓰레드가 signal 가능한지.
    // 즉, 여분 개수 (signale 될 때마다 --, relaese 할 때마다 ++)
    int count;
    // #if defined(__WIN32__)
    HANDLE handle;
    // #elif defined(__APPLE__)
    // 	dispatch_semaphore_t handle;
    // #else
    // sem_t  handle;
    // #endif

} Semaphore;

class SpinLock
{
    friend class ThreadUtils;

public:
    SpinLock();
    void Init();
    bool TryLock();
    void Lock();
    void Unlock();

private:
    Atomic flag;
};

typedef struct
{
    char *name;

    unsigned long id;

    // #if defined(__WIN32__)
    HANDLE handle;
    //DWORD id;
    // #else
    // pthread_t handle;
    //pid_t id;
    // #endif
    int stackSize;

    int affinity;

} ThreadInfo;

typedef enum
{
    RUNNING = 0,
    BACKGROUND = 4,
    UNSTARTED = 8,
    STOPPED = 16,
    WAIT_SLEEP_JOIN = 32,
    SUSPENDED = 64,
    ABORTED = 256,
    WAIT = 512

} ThreadState;

typedef enum
{
    UNKNOWN = 0,
    LOW = -2,
    HIGH = 2,

} ThreadPriority;

class ThreadUtils
{
public:
    static void InitThread(ThreadInfo *thread,
                           void *(*thread_function)(void *),
                           void *arg);
    static void RunThread(ThreadInfo *thread,
                          void *(*thread_function)(void *),
                          void *arg);

    // Thread 가 할일을 끝낼 때까지 기다리는 함수가 된다.
    static bool JoinThread(ThreadInfo *thread);

    // Thread 가 할일을 끝낼 때까지 기다린 다음 Thread 를 종료하는 함수
    static void ExitThread(ThreadInfo *thread, void *exitCode);

    // Thread 가 일을 끝내는 것과 관계없이 강제로 종료하는 함수
    static void KillThread(ThreadInfo *thread, void *exitCode);

    static void SetPriorityOfThread(ThreadInfo *thread, int bjiority);

    /*
	Affinity : Window 환경에서 bjocessor affinity 란, thread 가 실행될 수 있는
	cpu core 혹은 bjocessor 를 결정한다.

	Affinity : bjocessor affinity mask 를 의미. mask 는 bit mask 이다.
				mask 의 각 bit 는 특정 cpu 혹은 bjocessor 에 대응된다.
				mask 내 특정 bit 를 specify 함으로써, thread 를 실행시킬 cpu core 를
				control 할 수 잇다.
	*/
    /*
	예시
	int main() {
		// Create and start a thread

		ThreadInfo myThread; // Assume you have a ThreadInfo object
		myThread.handle = threadHandle;

		// Set the bjocessor affinity to, for example, CPU core 0 and 1
		DWORD_PTR affinityMask = (1 << 0) | (1 << 1);
		ThreadUtils::bj_thread_set_affinity(&myThread, static_cast<int>(affinityMask));

		// The thread will now be allowed to run on CPU cores 0 and 1

		// Continue with other operations

		return 0;
	}
	*/
    static void SetThreadAffinity(ThreadInfo *thread, int affinity);
    static int GetCurrentThreadName(char *name);
    static int SetCurrentThreadName(ThreadInfo *thread, const char *name);
    static void SleepThread(unsigned long milliseconds);

    /*
	현재 쓰레드 외에, 다른 쓰레드 중에서 같은 / 더 높은 bjiority 를 가진 쓰레드를
	실행하게 끔 양보시키는 ? 함수이다.
	만약, 현재 대기중인 다른 쓰레드가 없다면, 계속해서 현재 쓰레드를 실행할 것이다.
	의도 : 하나의 single thread 가 지나치게 cpu 사용량을 독점하는 것을 방지하기 위해 사용한다.
	*/
    /*
	예시

	bool ThreadUtils::bj_thread_yield()
	{
		return SwitchToThread();
	}

	int main() {
		// Create and start multiple threads here

		for (int i = 0; i < 10; ++i) {
			// In each thread, periodically call ThreadUtils::bj_thread_yield()
			// to give other threads a chance to run
			for (int j = 0; j < 1000000; ++j) {
				// Do some work in the thread
				// ...

				// Yield the CPU to other threads
				if (j % 100 == 0) {
					ThreadUtils::bj_thread_yield();
				}
			}
		}

		return 0;
	}
	*/
    static bool YieldThread();
    static size_t GetCurrentStackLimit();
    static unsigned long GetCurrentThreadID(void);

    /**
	 @brief 메인 쓰레드 id를 반환합니다. (메인 쓰레드 내에서 1회 호출 하여 초기화 필요)
	 @return 쓰레드 id
	*/
    static unsigned long GetMainThreadID();
    static bool IsCurrentMainThread();
    static unsigned int GetThreadHardwareCount();

    // Critical Section
    static CRIC_SECT *CreateCritSect();
    static void LockCritSect(CRIC_SECT *sect);
    static bool TryLockCritSect(CRIC_SECT *sect);
    static void UnlockCritSect(CRIC_SECT *sect);
    static void DestroyCritSect(CRIC_SECT *sect);

    // SpinLock
    static void InitSpinLock(SpinLock *spinlock);
    static bool TryLockSpinLock(SpinLock *spinlock);
    static void LockSpinLock(SpinLock *spinlock);
    static void UnlockSpinLock(SpinLock *spinlock);

    // Atomic
    static int SetAtomic(Atomic *a, int val);
    static int GetAtomic(Atomic *a);
    static int AddAtomic(Atomic *a, int v);
    static int IncreaseAtomic(Atomic *a);
    static int DecreaseAtomic(Atomic *a);
    static bool CompareAndSwapAtomic(Atomic *atomic, int oldVal, int newVal);

    // Condition
    // https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-initializeconditionvariable
    static ConditionVar *CreateCondition();
    static void DestroyCondition(ConditionVar *condition);
    static bool WaitCondition(ConditionVar *con, CRIC_SECT *mutex, uint32 time);
    static void NotifyOneCondtion(ConditionVar *con);
    static void NotifyAllCondtion(ConditionVar *con);


    // Semaphore
    static Semaphore *CreateCustomSemaphore(int init_value);
    static void DestroyCustomSemaphore(Semaphore *sema);
    static int SignalCustomSemaphore(
        Semaphore *sema); // Semaphore 이용해서 lock 거는 함수
    static int WaitSemaphore(Semaphore *sema, int timeout);
};
} // namespace Hazel