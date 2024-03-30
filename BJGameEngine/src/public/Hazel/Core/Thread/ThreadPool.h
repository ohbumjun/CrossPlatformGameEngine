#pragma once

#include "hzpch.h"
#include "Thread.h"
#include "ThreadTask.h"

namespace Hazel
{
class ThreadPool;
class ThreadPoolWorker
{
    using PoolTask = ThreadTask<void(void *)>;

public:
    ThreadPoolWorker(ThreadPool &s, int index);

    virtual ~ThreadPoolWorker();

    const char *GetName();

    Thread *GetThread();

    void ExecuteThread(void *arg = nullptr);

    void Finalize();


private:
    std::string m_Name;

    ThreadPool *m_ThreadPool;

    Thread m_Thread;

    static void Execute(void *ptr)
    {
        ThreadPoolWorker *worker = reinterpret_cast<ThreadPoolWorker *>(ptr);
        worker->ExecuteThread(worker);
    }
};

class ThreadPool
{
    using PoolTask = ThreadTask<void(void *)>;

    typedef void (*Procedure)(void *);

    friend class ThreadPoolWorker;

public:
    // �����Ϸ����� default constructor ������ ������ ����� ����ϴ� ����
    // Ư�� ������� ��ü ������ ���� ���� �� ����Ѵ�. �ʿ��� ���� ���� �ݵ�� �Ѱ���߸� �����ǰ� �ϰ��� �Ѵ�.
    ThreadPool() = delete;

    ThreadPool(size_t count, const char *name = "Thread");

    virtual ~ThreadPool();

    size_t GetTaskCount();

    size_t GetWorkerCount();

    void ClearThreads();

    //task�� �ܺο��� �־���� ������ ���� å�ӵ� �ܺο� ����
    void AddPoolTask(PoolTask *t, void *arg = nullptr);

    // finished task �� task count �� �ɶ����� ��ٸ��� �Լ�
    void Wait(int32 taskCount);

    void WaitAllThreads();

    inline const std::string &GetName()
    {
        return m_Name;
    }

private:
    void createPoolWorker(size_t count, const char *name);

    bool loadTaskFromPool(PoolTask *&out, void *&outArg);

    std::vector<ThreadPoolWorker *> m_PoolWorkers;

    CRIC_SECT *m_CricSect;

    ConditionVariable *m_Condition;

    // m_Task[i] �� m_Args[i] �� 1:1 ������ �ȴ�.
    std::queue<PoolTask *> m_PoolTasks;
    std::queue<void *> m_Args;

    Atomic m_FinishTaskCount;

    Atomic m_TotalTaskCount;

    bool m_IsStop;

    std::string m_Name;
};

} // namespace Hazel