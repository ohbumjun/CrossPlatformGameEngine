#pragma once

#include <process.h>
#include <thread>

#define THREAD_WAIT_INFINITE (~(uint32)0)
#define THREAD_WAIT_TIMEDOUT 1
namespace Hazel
{

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
    LOW = -2,
    UNKNOWN = 0,
    HIGH = 2,

} ThreadPriority;

typedef struct
{
    // #if defined(__WIN32__)
    CONDITION_VARIABLE handle;
    // #else
    // pthread_cond_t handle;
    // #endif

} ConditionVariable;

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

    Atomic(const Atomic &o);

    int GetVal();

    volatile int atomicVal;
};


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
