#pragma once

#include "ThreadVars.h"

class ThreadUtils
{
public:
    // Thread 가 할일을 끝낼 때까지 기다리는 함수가 된다.
    static bool JoinThread(ThreadInfo *thread);

    // Thread 가 할일을 끝낼 때까지 기다린 다음 Thread 를 종료하는 함수
    static void ExitThread(ThreadInfo *thread, void *exitCode);


    static void InitThread(ThreadInfo *thread,
                           void *(*thread_function)(void *),
                           void *arg);
    static void RunThread(ThreadInfo *thread,
                          void *(*thread_function)(void *),
                          void *arg);

    // Thread 가 일을 끝내는 것과 관계없이 강제로 종료하는 함수
    static void KillThread(ThreadInfo *thread, void *exitCode);

    static void SetPriorityOfThread(ThreadInfo *thread, int bjiority);

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


    /**
	 @brief 메인 쓰레드 id를 반환합니다. (메인 쓰레드 내에서 1회 호출 하여 초기화 필요)
	 @return 쓰레드 id
	*/
    static unsigned long GetMainThreadID();
    static bool IsCurrentMainThread();
    static unsigned int GetThreadHardwareCount();

    // Atomic
    static int SetAtomic(Atomic *a, int val);
    static int GetAtomic(Atomic *a);
    static int AddAtomic(Atomic *a, int v);
    static int IncreaseAtomic(Atomic *a);
    static int DecreaseAtomic(Atomic *a);
    static bool CompareAndSwapAtomic(Atomic *atomic, int oldVal, int newVal);

    // Condition
    // https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-initializeconditionvariable
    static ConditionVariable *CreateCondition();
    static void DestroyCondition(ConditionVariable *condition);
    static bool WaitCondition(ConditionVariable *con,
                              CRIC_SECT *mutex,
                              uint32 time);
    static void NotifyOneCondtion(ConditionVariable *con);
    static void NotifyAllCondtion(ConditionVariable *con);

    // SpinLock
    static void InitSpinLock(SpinLock *spinlock);
    static bool TryLockSpinLock(SpinLock *spinlock);
    static void LockSpinLock(SpinLock *spinlock);
    static void UnlockSpinLock(SpinLock *spinlock);

    // Critical Section
    static CRIC_SECT *CreateCritSect();
    static void LockCritSect(CRIC_SECT *sect);
    static bool TryLockCritSect(CRIC_SECT *sect);
    static void UnlockCritSect(CRIC_SECT *sect);
    static void DestroyCritSect(CRIC_SECT *sect);


    // Semaphore
    static Semaphore *CreateCustomSemaphore(int init_value);
    static void DestroyCustomSemaphore(Semaphore *sema);
    static int SignalCustomSemaphore(
        Semaphore *sema); // Semaphore 이용해서 lock 거는 함수
    static int WaitSemaphore(Semaphore *sema, int timeout);
};
} // namespace Hazel