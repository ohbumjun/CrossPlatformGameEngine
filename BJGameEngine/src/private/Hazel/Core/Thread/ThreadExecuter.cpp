#include "Hazel/Core/Thread/ThreadExecuter.h"
#include "Hazel/Core/Thread/Thread.h"
#include "Hazel/Core/Thread/ThreadUtil.h"
#include "Hazel/Utils/TimeStep.h"
#include "Hazel/Utils/TimeUtil.h"


namespace Hazel
{

ThreadExecuter::ThreadHandle *ThreadExecuter::_mainThreadHandle = nullptr;
ThreadExecuter *ThreadExecuter::_mainExecuter = nullptr;

std::vector<uint64> ThreadExecuter::_threadIds;
std::unordered_map<uint64, ThreadExecuter::ThreadHandle *>
    ThreadExecuter::_threadHandles;
std::unordered_map<uint64, RingBuffer<ThreadExecuter::Task *> *>
    ThreadExecuter::_threadTasks;
std::unordered_map<uint64, ThreadExecuter::Desc>
    ThreadExecuter::_threadDescriptions;
std::unordered_map<uint64, Thread *> ThreadExecuter::_threads;
std::unordered_map<uint64, CRIC_SECT *> ThreadExecuter::_threadMutexes;
std::unordered_map<uint64, ConditionVar *> ThreadExecuter::_threadConditions;
static uint64 s_LastThreadId = 0;
static CRIC_SECT *s_mutex;

ThreadExecuter::ThreadHandle *ThreadExecuter::Init()
{
    s_mutex = ThreadUtils::CreateCritSect();

    // LV_CHECK(nullptr == _handle, "Duplicate calls");

    _mainExecuter = new ThreadExecuter("Main Thread Executuer");
    _mainThreadHandle = new ThreadExecuter::ThreadHandle(*_mainExecuter);

    return _mainThreadHandle;
}

void ThreadExecuter::Finalize()
{
    std::vector<uint64> releaseIds(_threadIds);
    for (size_t i = 0, max = releaseIds.size(); i < max; ++i)
    {
        releaseThread(releaseIds[i]);
    }

    delete _mainThreadHandle;
    delete _mainExecuter;

    ThreadUtils::DestroyCritSect(s_mutex);
}

void ThreadExecuter::runThreadTask(void *argThreadId)
{
    uint64 currentId = reinterpret_cast<uint64>(argThreadId);

    const uint32 start = TimeUtil::GetTimeMiliSec();

    ThreadUtils::LockCritSect(s_mutex);
    ThreadHandle *targetThread = _threadHandles[currentId];
    RingBuffer<Task *> *threadTasks = _threadTasks[currentId];
    ThreadUtils::UnlockCritSect(s_mutex);

    // 비어있는 상태에서 시작하지 않도록 대기시킬 것이다.
    while (threadTasks->Count() <= 0){}

    // 10초(10ms * 1000) 유지
    // 즉, 해당 쓰레드가 일을 하고 나서 10초까지만 살아있게 한다는 것이다.
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

    // 메인쓰레드로 하여금 비동기로, Frame 마지막에 현재 Thread 정보를
    // 해제할 수 있도록 한다.
    _mainExecuter->AsyncExecute([currentId]() { releaseThread(currentId); });
}

uint64 ThreadExecuter::createThread(const char *description)
{
    ThreadUtils::LockCritSect(s_mutex);

    uint64 id = s_LastThreadId;

    // 기존에 없단 Thread ID 를 만들 때까지 ID 를 증가시킨다.
    while (_threadTasks.count(id) > 0)
    {
        id++;
    }
    s_LastThreadId = id;

    // 0. Id
    _threadIds.push_back(id);

    // 1. Handle
    _threadHandles.insert({id, new ThreadExecuter::ThreadHandle(id)});

    // 2. Description
    Desc desc;
    desc.desc = description;
    _threadDescriptions.insert({id, std::move(desc)});

    // 3. Queue
    _threadTasks.insert({id, new RingBuffer<Task *>()});

    // 4. Mutexes
    _threadMutexes.insert({id, ThreadUtils::CreateCritSect()});

    // 5. Condition
    _threadConditions.insert({id, ThreadUtils::CreateCondition()});

    // 6. Thread
    // 0은 메인 큐이기 때문에 쓰레드를 생성하지 않음
    // 이러한 로직 말고, 다른 방식으로 메인쓰레드 id 를 정의했으면 좋겠다.
    if (0 != id)
    {
        Thread *thread = new Thread();
        thread->SetName(_threadDescriptions[id].desc.c_str());

        unsigned long threadId = thread->GetThreadID();

        thread->Start(runThreadTask, reinterpret_cast<void *>(id));

        _threads.insert({id, thread});
    }

    ThreadUtils::UnlockCritSect(s_mutex);

    return id;
}

void ThreadExecuter::releaseThread(uint64 id)
{
    ThreadUtils::LockCritSect(s_mutex);

    // 6. Thread
    // if (_threads.ContainsKey(id))
    if (_threads.count(id) > 0)
    {
        Thread *thread = _threads[id];
        delete _threads[id];
        _threads.erase(id);
    }

    // 5. Condition
    ThreadUtils::DestroyCondition(_threadConditions[id]);
    _threadConditions.erase(id);

    // 4. Mutexes
    ThreadUtils::DestroyCritSect(_threadMutexes[id]);
    _threadMutexes.erase(id);

    // 3. Queue
    RingBuffer<Task *> *queue = _threadTasks[id];
    while (0 < queue->Count())
    {
        Task *item;
        if (queue->Dequeue(item))
        {
            delete item;
        }
    }
    delete queue;
    _threadTasks.erase(id);

    // 2. Description
    _threadDescriptions.erase(id);

    // 1. Handle
    if (_threadHandles.count(id) > 0)
    {
        delete _threadHandles[id];
        _threadHandles.erase(id);
    }

    // 0. Id
    // _threadIds.Remove(id);
    _threadIds.erase(std::find(_threadIds.begin(), _threadIds.end(), id));

    ThreadUtils::UnlockCritSect(s_mutex);
}

RingBuffer<ThreadExecuter::Task *> *ThreadExecuter::getThreadTasks(uint64 id)
{
    RingBuffer<Task *> *result = nullptr;

    if (_threadTasks.count(id) > 0)
    {
        result = _threadTasks[id];
    }

    return result;
}

const char *ThreadExecuter::getThreadDesc(uint64 id)
{
    // if (_threadTasks.ContainsKey(id))
    if (_threadTasks.count(id) > 0)
    {
        // return _threadDescriptions.ContainsKey(id) ? _threadDescriptions[id].c_str() : "";
        return _threadDescriptions.count(id) > 0
                   ? _threadDescriptions[id].desc.c_str()
                   : "";
    }
    else
    {
        return nullptr;
    }
}

ThreadExecuter::ThreadExecuter(uint64 id)
    : _threadId(id), _description(getThreadDesc(_threadId))
{
}

// TODO : 같은 description 으로 들어오게 되면, 기존의 thread 에 task 를 추가하는
// 방식으로 진행하는 것도 좋을 것 같다.
// ThreadExecuter::Find() 함수 같은 거를 만들어서, 진행하면 좋을 것 같다.
// 안 그러면, main 함수 loop 에서 계속해서 몇백개의 thread 를 생성하는 과정을
// 거치게 된다.
ThreadExecuter::ThreadExecuter(const char *description)
    : _threadId(createThread(description)),
      _description(getThreadDesc(_threadId))
{
}

void ThreadExecuter::AsyncExecute(std::function<void()> &&function)
{
    async(new Task(std::move(function)));
}

void ThreadExecuter::SyncExecute(std::function<void()> &&function)
{
    // Main thread 에서 Main Dispatch Queue 에 Sync 하는 경우 교착상태에 빠지게 됨
    ThreadUtils::LockCritSect(s_mutex);
    CRIC_SECT *targetMutex = _threadMutexes[_threadId];
    ConditionVar *targetCondition = _threadConditions[_threadId];
    ThreadUtils::UnlockCritSect(s_mutex);

    ThreadUtils::LockCritSect(targetMutex);
    async(new Task(std::move(function)));

    // ex) Main Thread -> ThreadExecuter("TEST1").Sync 를 호출하면
    // Main 이 해당 ThreadExecuter 의 condition 을 wait 하고
    // TEST1 에 해당하는 쓰레드가 일을 끝내면, 해당 condition 에 notify 를 해서
    // Main 을 깨운다.
    ThreadUtils::WaitCondition(targetCondition,
                               targetMutex,
                               THREAD_WAIT_INFINITE);

    ThreadUtils::UnlockCritSect(targetMutex);
}

void ThreadExecuter::async(ThreadExecuter::Task *Task) const
{
    RingBuffer<ThreadExecuter::Task *> *items = getThreadTasks(_threadId);

    if (nullptr != items)
    {
        items->Enqueue(Task);

    }
    else
    {
        bool h = true;
    }
}

bool ThreadExecuter::ThreadHandle::ExecuteHandle()
{
    bool executed = false;
    RingBuffer<Task *> *items = getThreadTasks(_selfId);

    if (nullptr != items)
    {
        bool notify = false;
        while (!items->IsEmpty())
        {
            if (items->Dequeue(_currentItem))
            {
#if defined(_DEBUG)
                const char *description = getThreadDesc(_selfId);
#endif
                _currentItem->_task();
                if (0 != _selfId)
                {
                    // _LOG("ThreadExecuter::Dequeue ([%llu]%s) / count %zu\n", _id, getDescription(_id), items->Count())
                }
                delete _currentItem;
                _currentItem = nullptr;

                executed = true;
            }
            else
            {
                break;
            }
            notify = true;
        }
        // if (_mutexes.ContainsKey(_id) && notify)
        if (_threadMutexes.count(_selfId) > 0 && notify)
        {
            ThreadUtils::LockCritSect(_threadMutexes[_selfId]);

            ThreadUtils::NotifyOneCondtion(_threadConditions[_selfId]);

            ThreadUtils::UnlockCritSect(_threadMutexes[_selfId]);
        }
    }
    return executed;
}

ThreadExecuter::ThreadHandle::~ThreadHandle()
{
    if (nullptr != _currentItem)
    {
        delete _currentItem;
        _currentItem = nullptr;
    }
}
} // namespace Hazel