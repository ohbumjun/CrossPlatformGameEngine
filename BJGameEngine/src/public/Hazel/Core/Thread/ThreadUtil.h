#pragma once

#include <process.h>
#include <thread>


#define bj_WAIT_INFINITE (~(uint32)0)
#define bj_WAIT_TIMEDOUT  1

struct CRIC_SECT
{
	CRITICAL_SECTION handle;
	// ��Ƽ������ ȯ�濡�� ,���� �����忡 ���� ���� �� �����Ƿ�
	// ����ȭ x
	volatile bool isInit;
};

struct Atomic
{
	Atomic() = default;

	Atomic(const Atomic& o);

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
	// ex. �ʱⰪ == 5
	// ���� ��� �����尡 signal ��������. 
	// ��, ���� ���� (signale �� ������ --, relaese �� ������ ++)
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
private :
	Atomic flag;
};

typedef struct
{
	char* name;

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

} Threadbjiority;

class ThreadUtils
{
public:
	static void bj_thread_init(ThreadInfo* thread, void* (*thread_function)(void*), void* arg);
	static void bj_thread_run(ThreadInfo* thread, void* (*thread_function)(void*), void* arg);
	static bool bj_thread_join(ThreadInfo* thread, void* result);

	// Thread �� ������ ���� ������ ��ٸ� ���� Thread �� �����ϴ� �Լ�
	static void bj_thread_exit(ThreadInfo* thread, void* exitCode);

	// Thread �� ���� ������ �Ͱ� ������� ������ �����ϴ� �Լ�
	static void bj_thread_kill(ThreadInfo* thread, void* exitCode);

	static void bj_thread_set_priority(ThreadInfo* thread, int bjiority);

	/*
	Affinity : Window ȯ�濡�� bjocessor affinity ��, thread �� ����� �� �ִ�
	cpu core Ȥ�� bjocessor �� �����Ѵ�.

	Affinity : bjocessor affinity mask �� �ǹ�. mask �� bit mask �̴�.
				mask �� �� bit �� Ư�� cpu Ȥ�� bjocessor �� �����ȴ�.
				mask �� Ư�� bit �� specify �����ν�, thread �� �����ų cpu core ��
				control �� �� �մ�.
	*/
	/*
	����
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
	static void bj_thread_set_affinity(ThreadInfo* thread, int affinity);
	static int  bj_current_thread_get_name(char* name);
	static int  bj_thread_set_name(ThreadInfo* thread, const char* name);
	static void bj_thread_sleep(unsigned long milliseconds);

	/*
	���� ������ �ܿ�, �ٸ� ������ �߿��� ���� / �� ���� bjiority �� ���� �����带
	�����ϰ� �� �纸��Ű�� ? �Լ��̴�.
	����, ���� ������� �ٸ� �����尡 ���ٸ�, ����ؼ� ���� �����带 ������ ���̴�.
	�ǵ� : �ϳ��� single thread �� ����ġ�� cpu ��뷮�� �����ϴ� ���� �����ϱ� ���� ����Ѵ�.
	*/
	/*
	����

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
	static bool bj_thread_yield();
	static size_t bj_thread_current_stack_limit();
	static unsigned long bj_thread_get_current_id(void);

	/**
	 @brief ���� ������ id�� ��ȯ�մϴ�. (���� ������ ������ 1ȸ ȣ�� �Ͽ� �ʱ�ȭ �ʿ�)
	 @return ������ id
	*/
	static unsigned long bj_main_thread_id();
	static bool bj_is_current_main_thread();
	static unsigned int bj_thread_get_hardware_count();

	// Critical Section
	static CRIC_SECT* bj_crit_sect_create();
	static void bj_crit_sect_lock(CRIC_SECT* sect);
	static bool bj_crit_sect_try_lock(CRIC_SECT* sect);
	static void bj_crit_sect_unlock(CRIC_SECT* sect);
	static void bj_crit_sect_destroy(CRIC_SECT* sect);

	// SpinLock
	static void bj_spin_init(SpinLock* spinlock);
	static bool bj_spin_try_lock(SpinLock* spinlock);
	static void bj_spin_lock(SpinLock* spinlock);
	static void bj_spin_unlock(SpinLock* spinlock);

	// Atomic
	static int  bj_atomic_set(Atomic* a, int val);
	static int  bj_atomic_get(Atomic* a);
	static int  bj_atomic_add(Atomic* a, int v);
	static int  bj_atomic_increase(Atomic* a);
	static int  bj_atomic_decrease(Atomic* a);
	static bool bj_atomic_compare_and_swap(Atomic* atomic, int oldVal, int newVal);

	// Condition
// https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-initializeconditionvariable
	static ConditionVar* bj_condition_create();
	static void bj_condition_destroy(ConditionVar* condition);
	static bool bj_condition_wait(ConditionVar* con, CRIC_SECT* mutex, uint32 time);
	static void bj_condition_notify_one(ConditionVar* con);
	static void bj_condition_notify_all(ConditionVar* con);


	// Semaphore
	static Semaphore* bj_semaphore_create(int init_value);
	static void bj_semaphore_destroy(Semaphore* sema);
	static int bj_semaphore_signal(Semaphore* sema);   // Semaphore �̿��ؼ� lock �Ŵ� �Լ�
	static int bj_semaphore_wait(Semaphore* sema, int timeout);
};

