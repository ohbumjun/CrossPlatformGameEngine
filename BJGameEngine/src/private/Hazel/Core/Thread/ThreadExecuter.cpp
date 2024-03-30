#include "Hazel/Core/Thread/ThreadExecuter.h"
#include "Hazel/Core/Thread/Thread.h"
#include "Hazel/Core/Thread/ThreadUtil.h"
#include "Hazel/Utils/TimeStep.h"
#include "Hazel/Utils/TimeUtil.h"


namespace Hazel
{

ThreadExecuterManager::ThreadHandle *ThreadExecuterManager::m_MainThreadHandle =
    nullptr;
uint64 ThreadExecuterManager::m_MainExecuterId;

std::vector<uint64> ThreadExecuterManager::_threadIds;
std::unordered_map<uint64, ThreadExecuterManager::ThreadExecuter *>
    ThreadExecuterManager::_threadExecuters;
static uint64 s_LastThreadId = 0;
static CRIC_SECT *s_mutex;

ThreadExecuterManager::ThreadExecuter::~ThreadExecuter()
{
    ThreadUtils::DestroyCondition(m_Condition);

    ThreadUtils::DestroyCritSect(m_Mutex);

    while (0 < m_Tasks->Count())
    {
        ExecuterTask *item;
        if (m_Tasks->Dequeue(item))
        {
            delete item;
        }
    }
    delete m_Tasks;

    delete m_Handle;
}

void ThreadExecuterManager::ThreadExecuter::AsyncExecute(
    std::function<void()> &&function)
{
    asyncExecute(new ExecuterTask(std::move(function)));
}

void ThreadExecuterManager::ThreadExecuter::SyncExecute(
    std::function<void()> &&function)
{
    // Main thread ���� Main Dispatch Queue �� Sync �ϴ� ��� �������¿� ������ ��
    ThreadUtils::LockCritSect(s_mutex);
    CRIC_SECT *targetMutex = _threadExecuters[m_ThreadId]->m_Mutex;
    ConditionVariable *targetCondition =
        _threadExecuters[m_ThreadId]->m_Condition;
    ThreadUtils::UnlockCritSect(s_mutex);

    ThreadUtils::LockCritSect(targetMutex);
    asyncExecute(new ExecuterTask(std::move(function)));

    // ex) Main Thread -> ThreadExecuter("TEST1").Sync �� ȣ���ϸ�
    // Main �� �ش� ThreadExecuter �� condition �� wait �ϰ�
    // TEST1 �� �ش��ϴ� �����尡 ���� ������, �ش� condition �� notify �� �ؼ�
    // Main �� �����.
    ThreadUtils::WaitCondition(targetCondition,
                               targetMutex,
                               THREAD_WAIT_INFINITE);

    ThreadUtils::UnlockCritSect(targetMutex);
}

void ThreadExecuterManager::ThreadExecuter::asyncExecute(
    ThreadExecuterManager::ExecuterTask *Task) const
{
    RingBuffer<ThreadExecuterManager::ExecuterTask *> *items =
        getExecuterTasks(m_ThreadId);

    if (nullptr != items)
    {
        items->Enqueue(Task);
    }
    else
    {
        bool h = true;
    }
}

ThreadExecuterManager::ThreadHandle *ThreadExecuterManager::Initialize()
{
    s_mutex = ThreadUtils::CreateCritSect();

    m_MainExecuterId = CreateExecuter("Main Thread Executuer");

    ThreadExecuterManager::ThreadExecuter *mainExecuter =
		_threadExecuters[m_MainExecuterId];

    m_MainThreadHandle = new ThreadExecuterManager::ThreadHandle(*mainExecuter);

    return m_MainThreadHandle;
}

void ThreadExecuterManager::Finalize()
{
    std::vector<uint64> releaseIds(_threadIds);
    for (size_t i = 0, max = releaseIds.size(); i < max; ++i)
    {
        releaseExecuter(releaseIds[i]);
    }

    delete m_MainThreadHandle;

    ThreadUtils::DestroyCritSect(s_mutex);
}

void ThreadExecuterManager::runExecuterTask(void *argThreadId)
{
    uint64 currentId = reinterpret_cast<uint64>(argThreadId);

    const uint32 start = TimeUtil::GetTimeMiliSec();

    ThreadUtils::LockCritSect(s_mutex);
    ThreadExecuter *threadExecuter = _threadExecuters[currentId]; 

    ThreadHandle *targetThread = threadExecuter->m_Handle;
    RingBuffer<ExecuterTask *> *threadTasks = threadExecuter->m_Tasks;

    ThreadUtils::UnlockCritSect(s_mutex);

    // ����ִ� ���¿��� �������� �ʵ��� ����ų ���̴�.
    while (threadTasks->Count() <= 0){}

    // 10��(10ms * 1000) ����
    // ��, �ش� �����尡 ���� �ϰ� ���� 10�ʱ����� ����ְ� �Ѵٴ� ���̴�.
    constexpr uint64 resetTime = 100;
    int64 currentTime = 100;
    while (0 < --currentTime)
    {
        if (targetThread->ExecuteHandle())
        {
            currentTime = resetTime;
        }

        ThreadUtils::SleepThread(10);
    }

    // ���ξ������ �Ͽ��� �񵿱��, Frame �������� ���� Thread ������
    // ������ �� �ֵ��� �Ѵ�.
    GetMain().AsyncExecute([currentId]() { releaseExecuter(currentId); });
}

uint64 ThreadExecuterManager::CreateExecuter(const char *description)
{
    ThreadUtils::LockCritSect(s_mutex);

    uint64 id = s_LastThreadId;

    // ������ ���� Thread ID �� ���� ������ ID �� ������Ų��.
    while (_threadExecuters.count(id) > 0)
    {
        id++;
    }
    s_LastThreadId = id;

    _threadIds.push_back(id);

    ThreadExecuter *newExecuter = new ThreadExecuter();
    newExecuter->m_ThreadId = id;
    newExecuter->m_Desc = description;
    newExecuter->m_Handle = new ThreadExecuterManager::ThreadHandle(id);
    newExecuter->m_Tasks = new RingBuffer<ExecuterTask *>();
    newExecuter->m_Mutex = ThreadUtils::CreateCritSect();
    newExecuter->m_Condition = ThreadUtils::CreateCondition();

    // 0�� ���� ť�̱� ������ �����带 �������� ����
    // �̷��� ���� ����, �ٸ� ������� ���ξ����� id �� ���������� ���ڴ�.
    if (0 != id)
    {
        Thread *thread = new Thread();
        thread->SetThreadName(newExecuter->m_Desc.c_str());

        unsigned long threadId = thread->GetThreadID();

        thread->StartThread(runExecuterTask, reinterpret_cast<void *>(id));

        newExecuter->m_Thread = thread;
    }

    _threadExecuters.insert(std::make_pair(id, newExecuter));

    ThreadUtils::UnlockCritSect(s_mutex);

    return id;
}

void ThreadExecuterManager::releaseExecuter(uint64 id)
{
    ThreadUtils::LockCritSect(s_mutex);

    if (_threadExecuters.count(id) > 0)
    {
        ThreadExecuter *executer = _threadExecuters[id];
        executer->~ThreadExecuter();
        _threadExecuters.erase(id);
    }

    _threadIds.erase(std::find(_threadIds.begin(), _threadIds.end(), id));

    ThreadUtils::UnlockCritSect(s_mutex);
}

RingBuffer<ThreadExecuterManager::ExecuterTask *> *ThreadExecuterManager::
    getExecuterTasks(
    uint64 id)
{
    RingBuffer<ExecuterTask *> *result = nullptr;

    if (_threadExecuters.count(id) > 0)
    {
        result = _threadExecuters[id]->m_Tasks;
    }

    return result;
}

const char *ThreadExecuterManager::getExecuterDesc(uint64 id)
{
    // if (_threadTasks.ContainsKey(id))
    if (_threadExecuters.count(id) > 0)
    {
        return _threadExecuters[id]->m_Desc.c_str();
    }
    else
    {
        return nullptr;
    }
}


bool ThreadExecuterManager::ThreadHandle::ExecuteHandle()
{
    bool executed = false;
    RingBuffer<ExecuterTask *> *items = getExecuterTasks(m_SelfId);

    if (nullptr != items)
    {
        bool notify = false;
        while (!items->IsEmpty())
        {
            if (items->Dequeue(m_CurrentItem))
            {
#if defined(_DEBUG)
                const char *description = getExecuterDesc(m_SelfId);
#endif
                m_CurrentItem->_task();
                delete m_CurrentItem;
                m_CurrentItem = nullptr;

                executed = true;
            }
            else
            {
                break;
            }
            notify = true;
        }
        if (_threadExecuters.count(m_SelfId) > 0 && notify)
        {
            ThreadUtils::LockCritSect(_threadExecuters[m_SelfId]->m_Mutex);

            ThreadUtils::NotifyOneCondtion(
                _threadExecuters[m_SelfId]->m_Condition);

            ThreadUtils::UnlockCritSect(_threadExecuters[m_SelfId]->m_Mutex);
        }
    }
    return executed;
}

ThreadExecuterManager::ThreadHandle::~ThreadHandle()
{
    if (nullptr != m_CurrentItem)
    {
        delete m_CurrentItem;
        m_CurrentItem = nullptr;
    }
}

} // namespace Hazel