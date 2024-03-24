#pragma once

#include "hzpch.h"
#include "Thread.h"
#include "ThreadTask.h"

namespace Hazel
{
class ThreadPool;
class ThreadPoolWorker
{
    using Task = ThreadTask<void(void *)>;

public:
    ThreadPoolWorker(ThreadPool &s, int index);

    virtual ~ThreadPoolWorker();

    void executeThread(void *arg = nullptr);

    void Finalize();

    const char *GetName();

    Thread *GetThread();

private:
    std::string m_Name;

    ThreadPool *m_ThreadPool;

    Thread m_Thread;

    static void Execute(void *ptr)
    {
        ThreadPoolWorker *worker = reinterpret_cast<ThreadPoolWorker *>(ptr);
        worker->executeThread(worker);
    }
};

class ThreadPool
{
    using Task = ThreadTask<void(void *)>;

    typedef void (*Procedure)(void *);

    friend class ThreadPoolWorker;

public:
    // 컴파일러에게 default constructor 조차도 만들지 말라고 얘기하는 문법
    // 특정 방식으로 객체 생성을 막고 싶을 때 사용한다. 필요한 인자 등을 반드시 넘겨줘야만 생성되게 하고자 한다.
    ThreadPool() = delete;

    ThreadPool(size_t count, const char *name = "Thread");

    virtual ~ThreadPool();

    //task를 외부에서 넣어줬기 때문에 해제 책임도 외부에 있음
    void AddPoolTask(Task *t, void *arg = nullptr);

    // finished task 가 task count 가 될때까지 기다리는 함수
    void Wait(int32 taskCount);

    void WaitAllThreads();

    void ClearThreads();

    size_t GetTaskCount();

    size_t GetWorkerCount();

    inline const std::string &GetName()
    {
        return m_Name;
    }

private:
    void createPoolWorker(size_t count, const char *name);

    bool loadTaskFromPool(Task *&out, void *&outArg);

    std::vector<ThreadPoolWorker *> m_PoolWorkers;

    CRIC_SECT *m_CricSect;

    ConditionVar *m_Condition;

    // m_Task[i] 와 m_Args[i] 는 1:1 대응이 된다.
    std::queue<Task *> m_PoolTasks;
    std::queue<void *> m_Args;

    Atomic m_FinishTaskCount;

    Atomic m_TotalTaskCount;

    bool m_IsStop;

    std::string m_Name;
};

} // namespace Hazel