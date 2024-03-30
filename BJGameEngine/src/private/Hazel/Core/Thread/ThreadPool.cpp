#include "Hazel/Core/Thread/ThreadPool.h"
#include "Hazel/Core/Thread/ThreadUtil.h"

namespace Hazel
{
ThreadPoolWorker::ThreadPoolWorker(ThreadPool &pool, int index)
    : m_ThreadPool(&pool)
{
    std::string workerName = pool.m_Name;
    workerName += std::to_string(index);

    m_Name = workerName;

    m_Thread.SetThreadName(const_cast<char *>(m_Name.c_str()));

    // Thread ��, Worker�� Execute �Լ��� ������ �� �ְ� �Ѵ�.
    m_Thread.StartThread(&ThreadPoolWorker::Execute, this);
}

ThreadPoolWorker::~ThreadPoolWorker()
{
}

// Worker ���� ȣ���ϴ� �Լ�
void ThreadPoolWorker::ExecuteThread(void *arg)
{
    void *taskArg = nullptr;
    PoolTask *task = nullptr;

    while (m_ThreadPool->loadTaskFromPool(task, taskArg))
    {
        if (task)
        {
            task->RunTask(taskArg);
            ThreadUtils::IncreaseAtomic(&m_ThreadPool->m_FinishTaskCount);
            taskArg = nullptr;
            task = nullptr;
        }
    }
}

void ThreadPoolWorker::Finalize()
{
    // ���� ���̰� ���� ? �켱 WaitForSingleObjectEx �� �Լ� �ǵ��� ��Ȯ�ϰ� �˾ƾ� �Ѵ�.
    m_Thread.JoinThread();
    m_Thread.StopThread();
}

const char *ThreadPoolWorker::GetName()
{
    return m_Thread.GetThreadName();
}

Thread *ThreadPoolWorker::GetThread()
{
    return &m_Thread;
}

ThreadPool::ThreadPool(size_t count, const char *name) : m_IsStop(false)
{
    m_CricSect = ThreadUtils::CreateCritSect();
    m_Condition = ThreadUtils::CreateCondition();
    m_PoolWorkers.reserve(count);

    ThreadUtils::SetAtomic(&m_TotalTaskCount, 0);
    ThreadUtils::SetAtomic(&m_FinishTaskCount, 0);

    createPoolWorker(count, name);
}

ThreadPool::~ThreadPool()
{
    m_IsStop = true;
    ThreadUtils::NotifyAllCondtion(m_Condition);

    for (size_t i = 0; i < m_PoolWorkers.size(); ++i)
    {
        // Worker �� ��� �ִ� Thread �� ���� �����Ѵ�.
        m_PoolWorkers[i]->Finalize();

        delete m_PoolWorkers[i];
    }

    ThreadUtils::DestroyCritSect(m_CricSect);
    ThreadUtils::DestroyCondition(m_Condition);
}

void ThreadPool::AddPoolTask(PoolTask *t, void *arg)
{
    ThreadUtils::LockCritSect(m_CricSect);
    ThreadUtils::AddAtomic(&m_TotalTaskCount, 1);

    m_PoolTasks.push(t);
    m_Args.push(arg);

    ThreadUtils::UnlockCritSect(m_CricSect);

    // ThreadPool.loadTask �� ȣ���Ͽ�, m_Condition ������ ���� waiting �ϰ� �ִ�
    // �����带 �ϳ� running ���·� �ٲ۴�.
    ThreadUtils::NotifyOneCondtion(m_Condition);
}

void ThreadPool::Wait(int32 taskCount)
{
    while (ThreadUtils::GetAtomic(&m_FinishTaskCount) < taskCount)
        ;

    ThreadUtils::AddAtomic(&m_FinishTaskCount, -taskCount);

    // taskCount ��ŭ ���� ����� ���̹Ƿ�, total���� ���ҽ����ش�.
    ThreadUtils::AddAtomic(&m_TotalTaskCount, -taskCount);
}

void ThreadPool::WaitAllThreads()
{
    int totalTaskCount = ThreadUtils::GetAtomic(&m_TotalTaskCount);

    while (ThreadUtils::GetAtomic(&m_FinishTaskCount) < totalTaskCount)
    {
        // totalTaskCount = ThreadUtils::GetAtomic(&m_TotalTaskCount);
    }

    ThreadUtils::AddAtomic(&m_FinishTaskCount, -totalTaskCount);
    ThreadUtils::AddAtomic(&m_TotalTaskCount, -totalTaskCount);
}

void ThreadPool::ClearThreads()
{
    ThreadUtils::LockCritSect(m_CricSect);

    while (m_PoolTasks.empty() == false)
        m_PoolTasks.pop();
    while (m_Args.empty() == false)
        m_Args.pop();

    ThreadUtils::SetAtomic(&m_TotalTaskCount, 0);
    ThreadUtils::SetAtomic(&m_FinishTaskCount, 0);

    ThreadUtils::UnlockCritSect(m_CricSect);
}

size_t ThreadPool::GetTaskCount()
{
    return m_PoolTasks.size();
}

size_t ThreadPool::GetWorkerCount()
{
    return m_PoolWorkers.size();
}

void ThreadPool::createPoolWorker(size_t count, const char *name)
{
    for (size_t i = 0; i < count; ++i)
    {
        m_PoolWorkers.push_back(
            new ThreadPoolWorker(*this, static_cast<int>(i)));
    }
}

// Workder ���� ȣ���ϴ� �Լ� �̴�.
// ��Ȯ�ϰԴ�, Worker �ȿ� �ִ� Thread ���� ȣ���ϴ� �Լ�
bool ThreadPool::loadTaskFromPool(PoolTask *&out, void *&outArg)
{
    if (m_IsStop)
    {
        return false;
    }

    ThreadUtils::LockCritSect(m_CricSect);

    // task �� ��� �� ������ ����Ѵ�.
    while (m_PoolTasks.empty() && !m_IsStop)
    {
        // ���Ǻ����� ���� ��ٸ���.
        bool conditionTrue = ThreadUtils::WaitCondition(m_Condition,
                                                        m_CricSect,
                                                        THREAD_WAIT_INFINITE);

        // ������ �´ٸ�, �������� ���̰�
        if (!conditionTrue)
        {
            // ������ ���� �ʴٸ�,
            // 1) �ش� Thread �� Blocking ���·� �����
            // 2) unlock ��Ű�� , ����ؼ� ����Ѵ�.
            ThreadUtils::UnlockCritSect(m_CricSect);
            return true;
        }
    }

    if (m_PoolTasks.size() > 0)
    {
        out = m_PoolTasks.front();
        m_PoolTasks.pop();

        outArg = m_Args.front();
        m_Args.pop();
    }

    ThreadUtils::UnlockCritSect(m_CricSect);

    if (m_IsStop)
    {
        return false;
    }

    return true;
}

} // namespace Hazel