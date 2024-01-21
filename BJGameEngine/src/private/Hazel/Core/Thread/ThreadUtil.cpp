#include "hzpch.h"
#include "Hazel/Core/Thread/ThreadUtil.h"

const DWORD MS_VC_EXCEPTION = 0x406D1388;
#pragma pack(push,8)  
typedef struct tagTHREADNAME_INFO
{
	DWORD dwType; // Must be 0x1000.  
	LPCSTR szName; // Pointer to name (in user addr space).  
	DWORD dwThreadID; // Thread ID (-1=caller thread).  
	DWORD dwFlags; // Reserved for future use, must be zero.  
} THREADNAME_INFO;
#pragma pack(pop)

static void get_last_error_message(const wchar_t* buf, size_t size)
{
	//wchar_t buf[256];
	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(wchar_t*)buf, (DWORD)size, NULL);

	wchar_t* ptr = (wchar_t*)buf;
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

Atomic::Atomic(const Atomic& o)
{

};

int Atomic::GetValue()
{
	return value;
};

void ThreadUtils::bj_thread_init(ThreadInfo* thread, void* (*thread_function)(void*), void* arg)
{
}

void ThreadUtils::bj_thread_run(ThreadInfo* thread, void* (*thread_function)(void*), void* arg)
{
	// ���� thread �� ���� os �κ��� handle ���� ���Ϲ޴´�.
	thread->handle = (HANDLE)_beginthreadex(
		NULL,                   // default security attributes
		0,						// use default stack Size  
		reinterpret_cast<unsigned(__stdcall*)(void*)>(thread_function), // Cast to correct signature
		arg,					// argument to thread function 
		0,                      // use default creation flags 
		(unsigned int*)&thread->id);			// returns the thread identifier 

	// ���� �Լ��� ���ο� thread �� ����� ������ �Ѵ�.
	// �ٷ� thread �� call back function �� �����ϵ��� ������ �ʴ´�. �����尡 ���� �����ϰ� ���� ������
	// �ü�� (�� ��ü�����δ� �����ٷ�)�� �����̱� �����̴�.

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
#pragma warning(disable: 6320 6322)  
		__try {
			RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
		}
#pragma warning(pop)  
	}
}

bool ThreadUtils::bj_thread_join(ThreadInfo* thread, void* result)
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
		- �ش� �����尡 signaled ���°� �� ������ ��ٸ���.
		  SleepConditionVariableCS  �� ��������, SleepConditionVariableCS  �Լ��� Ư�� ������ ������ ������ wait �ϰ� �ϴ� ���̶��
		  WaitForSingleObjectEx �� ���� general �� ��������, Ư�� ���ǿ� ������ ���� �ƴ϶�� ������ ���̰� �ִ�.
		*/
		WaitForSingleObjectEx(thread->handle, INFINITE, FALSE);
	}
	return true;
}

void ThreadUtils::bj_thread_exit(ThreadInfo* thread, void* exitCode)
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

void ThreadUtils::bj_thread_kill(ThreadInfo* thread, void* exitCode)
{
	if (TerminateThread(thread->handle, (DWORD)exitCode) == false)
	{
		wchar_t buf[256];
		get_last_error_message(buf, sizeof(buf));
		THROW("TerminateThread Error Code %ls", buf);
	}
}

void ThreadUtils::bj_thread_set_priority(ThreadInfo* thread, int priority)
{
	// In Windows, thread priorities range from THREAD_PRIORITY_IDLE(lowest) to THREAD_PRIORITY_TIME_CRITICAL(highest).
	SetThreadPriority(thread->handle, priority);
}

void ThreadUtils::bj_thread_set_affinity(ThreadInfo* thread, int affinity)
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

int ThreadUtils::bj_current_thread_get_name(char* name)
{
	if (ThreadUtils::bj_is_current_main_thread())
	{
		strcpy_s(name, strlen("Main Thread") + 1, "Main Thread");
		return 0;
	}
	else
	{
		wchar_t* threadName = NULL;

		HRESULT hr = GetThreadDescription(GetCurrentThread(), &threadName);

		if (SUCCEEDED(hr))
		{
			// wchar �� utf-8 ���� ���ڿ� ũ�⸦ �����Ѵ�.
			int size = WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS, threadName, -1, NULL, NULL, NULL, NULL);

			// wchar type �� threadName ���ڿ���, utf-8 type�� name ���� ��ȯ���ش�.
			WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS, threadName, -1, name, size, NULL, NULL);

			// threadName �� �Ҵ�� �޸𸮸� �������ش�. 
			LocalFree(threadName);
		}

		return hr;
	}
}

int ThreadUtils::bj_thread_set_name(ThreadInfo* thread, const char* name)
{
	return SetThreadDescription(thread->handle, (PCWSTR)name);
}

void ThreadUtils::bj_thread_sleep(unsigned long milliseconds)
{
	Sleep(milliseconds);
}

bool ThreadUtils::bj_thread_yield()
{
	return SwitchToThread();
}

size_t ThreadUtils::bj_thread_current_stack_limit()
{
	// ULONG_PTR : unsigned long ���·�, �޸� �ּҸ� �����ϴ� ���� type ���� ���� ���δ�.
	ULONG_PTR stackBase = 0;

	ULONG_PTR stackLimit = 0;

	GetCurrentThreadStackLimits(&stackLimit, &stackBase);

	// ���� thread �� ������ �ִ� stack �� size �� �������ش�. 
	// stackBase	: top of stack		(highest memory address in stack)
	// stackLimit   : bottom of stack	(lowest memory address in stack)
	// ���� : stack �� high -> low �������� �޸𸮸� �Ҵ��ذ���.
	return (size_t)(stackBase - stackLimit);
}

unsigned long ThreadUtils::bj_thread_get_current_id(void)
{
	return ((unsigned long)GetCurrentThreadId());
}

bool ThreadUtils::bj_is_current_main_thread()
{
	return ThreadUtils::bj_main_thread_id() == ThreadUtils::bj_thread_get_current_id();
}

/*
���� ������ ������ 1ȸ ȣ���Ͽ� �ʱ�ȭ�� �ʿ䰡 �ִ�.
*/
unsigned long ThreadUtils::bj_main_thread_id()
{
	static unsigned long mainThreadId = bj_thread_get_current_id();

	return mainThreadId;
}

unsigned int ThreadUtils::bj_thread_get_hardware_count()
{
	return std::thread::hardware_concurrency();
}

CRIC_SECT* ThreadUtils::bj_crit_sect_create()
{
	CRIC_SECT* sect = (CRIC_SECT*)malloc(sizeof(CRIC_SECT));

	InitializeCriticalSection(&sect->handle);

	sect->isInit = true;

	return sect;
};

void ThreadUtils::bj_crit_sect_lock(CRIC_SECT* sect)
{
	if (sect->isInit == false)
	{
		InitializeCriticalSection(&sect->handle);
		sect->isInit = true;
	}

	EnterCriticalSection(&sect->handle);
}

bool ThreadUtils::bj_crit_sect_try_lock(CRIC_SECT* sect)
{
	if (sect == nullptr)
	{
		THROW("sect is null");
	}

	// Mutex
	//return WaitForSingleObject(mutex->handle, 0) == WAIT_OBJECT_0;

	return TryEnterCriticalSection(&sect->handle) != 0;
}

void ThreadUtils::bj_crit_sect_unlock(CRIC_SECT* sect)
{
	LeaveCriticalSection(&sect->handle);
}

void ThreadUtils::bj_crit_sect_destroy(CRIC_SECT* sect)
{
	DeleteCriticalSection(&sect->handle);
	free(sect);
}

void ThreadUtils::bj_spin_init(SpinLock* spinlock)
{
	// �ش� spinlock �� atomic value �� 0 ���� �ʱ�ȭ
	bj_atomic_set(&spinlock->flag, 0);
}

bool ThreadUtils::bj_spin_try_lock(SpinLock* spinlock)
{
	// bj_spin_init �ÿ� '0'���� ���� 
	// old : 0
	// new : 1

	// ���� spinlock ���� '0'�̾��ٸ�, '1' �� update �ǰ� true ����
	//					  '1'�̾��ٸ�, '1' �� update ���� �ʰ� false �� ����
	return !bj_atomic_compare_and_swap(&spinlock->flag, 0, 1);
}

void ThreadUtils::bj_spin_lock(SpinLock* spinlock)
{
	// bj_spin_try_lock ���� false �� ������ ������ ���� �ݺ�
	// ��, spin_lock ���� '1' �϶����� ���� �ݺ� 
	//     �ٸ� �����忡�� �ش� ���� '1' �� ������ ������ ���� ���
	while (bj_spin_try_lock(spinlock));
}

void ThreadUtils::bj_spin_unlock(SpinLock* spinlock)
{
	// bj_spin_init �ÿ� '0'���� ���� 
	// old : 1
	// new : 0

	// ���� spinlock ���� '1'�̾��ٸ�, '0' �� update �ǰ� true ����
	//					  '0'�̾��ٸ�, '0' �� update ���� �ʰ� false �� ����
	bj_atomic_compare_and_swap(&spinlock->flag, 1, 0);
}

int ThreadUtils::bj_atomic_set(Atomic* a, int val)
{
	// _InterlockedExchange : atomic exchange ������ �����ϴ� �Լ�
	return _InterlockedExchange((long*)&a->value, val);
}

int ThreadUtils::bj_atomic_get(Atomic* a)
{
	int r;

	do
	{
		r = a->value;

		// bj_atomic_compare_and_swap(a, r, r)
		// false ? : a->value �� r �� �ٸ��ٴ� �ǹ�
		// true  ? : a->value �� r �� ���ٴ� �ǹ�
		// ���� �����忡�� �ش� atomic value �� ���� �����Ѵٰ� �����غ���.
		// a->value �� 1�̾���. 

		// A Thread : r = a->value; ������, ����, bj_atomic_compare_and_swap �� �����ϱ� ��. r �� 1
		// B Thread : �� ���̿� atomic add �� ��. a->value �� 2;

		// ��. A Thread �� bj_atomic_compare_and_swap �� ���ؼ� ���� atomic value �� '1' �� update �ϰ��� �Ѵ�.
		// A Thread : bj_atomic_compare_and_swap(a(2), r(1), r(1)) => 2�� 1 �� �ٸ���. ���� return ���� false
		// �ٽ� ������ �õ��Ѵ�

		// r = a->value (2)
		// A Thread : atomic->value �� '2' �� update �õ��Ѵ�.
		// bj_atomic_compare_and_swap(a(2), r(2), r(2)) => 2�� 2 �� ����. ���� return true
	} while (!bj_atomic_compare_and_swap(a, r, r));

	return r;
}

int ThreadUtils::bj_atomic_add(Atomic* a, int v)
{
	/*
	_InterlockedExchangeAdd �� _InterlockedIncrement �� ������

	1) _InterlockedExchangeAdd
	- ��� : Ư������ŭ ���� (�� 1�� �ƴϾ �ȴ�)
	- ���� : add ���� origin value

	2) _InterlockedIncrement
	- ��� : 1 �� ����
	- ���� : add �� origin value
	*/
	return _InterlockedExchangeAdd((long*)&a->value, v);
}

int ThreadUtils::bj_atomic_increase(Atomic* a)
{
	return _InterlockedIncrement((long*)&a->value);
}

int ThreadUtils::bj_atomic_decrease(Atomic* a)
{
	return _InterlockedDecrement((long*)&a->value);
}

bool ThreadUtils::bj_atomic_compare_and_swap(Atomic* atomic, int oldVal, int newVal)
{
	// ���    : Ư�� �� ~ atomic ���� ���Ѵ�. ���� �����ϴٸ� ���ο� value �� replace �Ѵ�. 
	// ���� �� : compare and swap ���� ���� ��
	// newValue : replace �ϰ��� �ϴ� ��
	// oldValue : ���� atomic->value �� ���ϰ��� �ϴ� ��.

	// True  ? : newValu �� update ����
	// False ? : update X
	return _InterlockedCompareExchange((long*)&atomic->value, (long)newVal, (long)oldVal) == (long)oldVal;

}

ConditionVar* ThreadUtils::bj_condition_create()
{
	ConditionVar* r = (ConditionVar*)malloc(sizeof(ConditionVar));

	InitializeConditionVariable(&r->handle);

	return r;
}

void ThreadUtils::bj_condition_destroy(ConditionVar* condition)
{
	free(condition);
}

bool ThreadUtils::bj_condition_wait(ConditionVar* con, CRIC_SECT* mutex, uint32 time)
{
	/*
	��Ƽ������ ȯ�濡��, �ټ��� �����尡 Ư�� condition �� true �� �ɶ����� ��ٸ��� �� �� ���Ǵ� �Լ��̴�.

	- ������� condition variable �� true �� �� ������ ��ٸ���(���ŷ), �� ���� critical section �� release �Ѵ�. (unlock)
	- �ٸ� running thread �� WakeConditionVariable �Լ��� ȣ���ϸ�, �ش� condition variable �� ���� ���ť�� �� �ִ�
	  ���ŷ �����带 �����.
	- �׸��� �ٽ� �ش� condition variable �� true ���� �ƴ����� �˻��Ѵ�.
	*/
	return SleepConditionVariableCS(&con->handle, &mutex->handle, time);
}

void ThreadUtils::bj_condition_notify_one(ConditionVar* con)
{
	WakeConditionVariable(&con->handle);
}

void ThreadUtils::bj_condition_notify_all(ConditionVar* con)
{
	WakeAllConditionVariable(&con->handle);
}

Semaphore* ThreadUtils::bj_semaphore_create(int init_count)
{
	Semaphore* r;
	r = (Semaphore*)malloc(sizeof(Semaphore));
	// init_count : ������ ��ü�� �ʱ� �����Դϴ�.�������� ���´� ������ 0���� ũ�� 0�� �� ��ȣ�� ���� ��� ��ȣ�� �����ϴ�.
	//				��� �Լ��� �������� ��ٸ��� �ִ� �����带 ������ ������ ���� 1�� �����մϴ�
	// lMaximumCount(32 * 1024) : ������ ��ü �ִ� ���� 
	// NULL : name
	r->handle = CreateSemaphore(NULL, init_count, 32 * 1024, NULL);
	r->count = init_count;

	if (r->handle == NULL)
	{
		THROW("Couldn't create semaphore");
	}

	return r;
}

void ThreadUtils::bj_semaphore_destroy(Semaphore* sema)
{
	if (sema != NULL)
	{
		if (sema->handle != NULL)
		{
			// semaphore �� ���õ� ���ҽ����� clear ���ش�.
			CloseHandle(sema->handle);
			sema->handle = 0;
		}
		free(sema);
	}
}

int ThreadUtils::bj_semaphore_signal(Semaphore* sema)
{
	if (sema == NULL)
	{
		THROW("Semaphore is null");
	}

	// InterlockedIncrement , InterlockedDecrement
	// - ��Ƽ������ ȯ�濡�� Ư�� ���� ���� 1����, 1 ���� �����ִ� �Լ�

	// InterlockedIncrement(&sema->count);
	// InterlockedIncrement((LONG volatile*)&sema->count);
	InterlockedIncrement((LONG*)&sema->count);

	// release one unit from semaphore
	if (ReleaseSemaphore(sema->handle, 1, NULL) == FALSE)
	{
		// InterlockedDecrement((LONG volatile*)&sema->count);
		InterlockedDecrement((LONG*)&sema->count);
		THROW("ReleaseSemaphore() failed");
	}
	return 0;
}

int ThreadUtils::bj_semaphore_wait(Semaphore* sema, int timeout)
{
	if (sema == NULL)
	{
		THROW("Semaphore is null");
	}

	int r;
	DWORD millisec;


	if (timeout == bj_WAIT_INFINITE)
	{
		millisec = INFINITE;
	}
	else
	{
		millisec = (DWORD)timeout;
	}

	// millisec ���� �������� ��ü�� ���� wait �� �Ѵ�.
	// ���
	// 1) semaphore �� ���� signal �� �ϰų� (..? release �ϰų��� �ƴѰ�?)
	// 2) timeout 
	switch (WaitForSingleObjectEx(sema->handle, millisec, FALSE))
	{
		// �ش� milisec �ð� �ȿ� ���������� signaled �Ǿ��ٴ� �ǹ�
	case WAIT_OBJECT_0:
		InterlockedDecrement((LONG*)&sema->count);
		r = 0;
		break;
	case WAIT_TIMEOUT:
		r = bj_WAIT_TIMEDOUT;
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
	_InterlockedExchange((long*)&flag.value, 0);
}

bool SpinLock::TryLock()
{
	// ���    : Ư�� �� ~ atomic ���� ���Ѵ�. ���� �����ϴٸ� ���ο� value �� replace �Ѵ�. 
	// ���� �� : compare and swap ���� ���� ��
	// newValue : replace �ϰ��� �ϴ� ��
	// oldValue : ���� atomic->value �� ���ϰ��� �ϴ� ��.

	// True  ? : newValu �� update ����
	// False ? : update X

	return !_InterlockedCompareExchange((long*)&flag.value, (long)0, (long)1) == (long)1;
	// return !bj_atomic_compare_and_swap(&flag, 0, 1);
}

void SpinLock::Lock()
{
	// bj_spin_try_lock ���� false �� ������ ������ ���� �ݺ�
	// ��, spin_lock ���� '1' �϶����� ���� �ݺ� 
	//     �ٸ� �����忡�� �ش� ���� '1' �� ������ ������ ���� ���
	while (TryLock());
}

void SpinLock::Unlock()
{	
	// bj_spin_init �ÿ� '0'���� ���� 
	// old : 1
	// new : 0

	// ���� spinlock ���� '1'�̾��ٸ�, '0' �� update �ǰ� true ����
	//					  '0'�̾��ٸ�, '0' �� update ���� �ʰ� false �� ����
	// bj_atomic_compare_and_swap(&flag.value, 1, 0);
	_InterlockedCompareExchange((long*)&flag.value, (long)1, (long)0) == (long)0;
}
