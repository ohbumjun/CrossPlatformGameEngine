
#include "Hazel/Core/Thread/Thread.h"

namespace Hazel
{
Thread::Thread() : m_Func(nullptr), m_ArgData(nullptr)
{
    m_Condition = ThreadUtils::CreateCondition();
    m_CricSect = ThreadUtils::CreateCritSect();
    m_Info.name = nullptr;

    ThreadUtils::SetAtomic(&m_ThreadState, ThreadState::UNSTARTED);
}

Thread::Thread(Func procedure, void *userData)
    : m_Func(procedure), m_ArgData(userData)
{
    m_Condition = ThreadUtils::CreateCondition();
    m_CricSect = ThreadUtils::CreateCritSect();
    m_Info.name = nullptr;

    ThreadUtils::SetAtomic(&m_ThreadState, ThreadState::UNSTARTED);
}

Thread::~Thread()
{
    ThreadUtils::DestroyCondition(m_Condition);
    ThreadUtils::DestroyCritSect(m_CricSect);
}

void Thread::Start(Func runnable, void *userData)
{
    m_Func = runnable;
    m_ArgData = userData;

    // 1. Invoke 함수를 실행하는 쓰레드 "형성"
    // 2. 해당 쓰레드는 스케줄러에 의해 관리됨. 이후 Thread::Invoke 함수 실행
    // 3. Thread::Invoke -> Thread.m_Func (자신에게 할당된 콜백함수) 실행
    ThreadUtils::RunThread(&m_Info, Invoke, this);
}

void Thread::Notify()
{
    // 여기에 왜 critical section lock 을 거는 거지..?
    // 2개 이상의 쓰레드가 일을 동시에 끝내서, notify 를 하려고 한다.
    // 그러면 2개의 blocking 쓰레드를 running 으로 만들어야 하는데
    // 동기화가 안되서 1개만 running 으로 깨워버릴 수 있기 때문이다.

    // 음... 그런데 여러 쓰레드에서 해당 Thread 객체에 접근이 가능한가..?
    // 운영체제 쓰레드 ~ Thread 객체.이렇게 하나씩 대응되는 것 아닌가..?
    ThreadUtils::LockCritSect(m_CricSect);
    m_IsWaiting = false;
    ThreadUtils::NotifyOneCondtion(m_Condition);
    ThreadUtils::UnlockCritSect(m_CricSect);
}

// 해당 m_Handle 에 대응되는 쓰레드를 종료시키는 함수
void Thread::Stop()
{
    ThreadUtils::LockCritSect(m_CricSect);
    ThreadUtils::SetAtomic(&m_ThreadState, ThreadState::STOPPED);

    // Thread 의 CloseHandle 함수를 호출한다.
    ThreadUtils::ExitThread(&m_Info, 0);
    ThreadUtils::UnlockCritSect(m_CricSect);
}

void Thread::Wait()
{
    ThreadUtils::LockCritSect(m_CricSect);

    m_IsWaiting = true;

    ThreadUtils::SetAtomic(&m_ThreadState, ThreadState::WAIT);

    // careful : https://www.justsoftwaresolutions.co.uk/threading/condition-variable-spurious-wakes.html

    while (m_IsWaiting)
    {
        // 해당 condition variable 이 signal 될 때까지 기다린다.
        // 즉, 다른 쓰레드가 해당 m_Condition 에 대해서 notify, wake 할 때까지 기다린다.
        // ?? : 그러면 여러 Thread 객체들 사이에 m_Condition 은 공유되어야 하는거 아닌가..?
        //      그런데 Thread 객체 생성자에서 각각 별도로 만들어주즌데 ??
        ThreadUtils::WaitCondition(m_Condition,
                                   m_CricSect,
                                   THREAD_WAIT_INFINITE);

        // 바로 위줄을 실행할 때, 현재 condition variable 이 만족되지 않은 상황이라면,
        // 위줄에서 현재 쓰레드를 블로킹 상태에 놓이게 된다.
        // 그러다가, 다른 running thread 에 의해 wake 되고, 조건이 만족되면, 그때 위줄을 실행하고
        // 그 다음줄을 실행하게 되는 원리이다.
        m_IsWaiting = false;
    }

    ThreadUtils::UnlockCritSect(m_CricSect);
}

void Thread::Join()
{
    ThreadUtils::SetAtomic(&m_ThreadState, ThreadState::WAIT_SLEEP_JOIN);

    if (!ThreadUtils::JoinThread(&m_Info))
    {
        THROW("Error ~ todo make more information");
    }
}

const char *Thread::GetName()
{
    return m_Info.name;
}

void Thread::SetName(char *name)
{
    m_Info.name = name;
}

void Thread::SetName(const char *name)
{
    m_Info.name = const_cast<char *>(name);
}

unsigned long Thread::GetThreadID()
{
    // Thread 객체는 그저 m_Handle 에 대응되는 thread 를 들고 있는 녀석일 뿐인데...
    // 막상 다른 쓰레드가 해당 Thread 객체에 대해서 GetThreadID 를 호출하면...
    // 정확한 정보를 얻을 수 있는 것일까 ?
    return ThreadUtils::GetCurrentThreadID();
}

int Thread::GetPriority()
{
    return m_Priority;
}

void Thread::SetPriority(int priority)
{
    m_Priority = priority;
    ThreadUtils::SetPriorityOfThread(&m_Info, priority);
}

void Thread::SetAffinity(int affinity)
{
    ThreadUtils::SetThreadAffinity(&m_Info, affinity);
}

void Thread::Sleep(unsigned long milliseconds)
{
    ThreadUtils::SleepThread(milliseconds);
}

void *Thread::Invoke(void *pThis)
{
    Thread *t = static_cast<Thread *>(pThis);

#if defined(__MACOSX__)
    // @donghun 맥에서는 해당 스레드 내부에서 직접 자신의 이름을 바꿀 수 있고, 외부에서 바꿀 방법이 없기 때문에 해당 함수는 외부에 노출시키지 않고 맥 전용으로 처리함
    // https://stackoverflow.com/questions/2369738/how-to-set-the-name-of-a-thread-in-linux-pthreads/7989973#7989973
    lv_thread_set_name(&t->GetHandle(), t->GetName());
#endif

    t->Run(t->m_ArgData);

    return nullptr;
}

void Thread::Run(void *args)
{
    ThreadUtils::SetAtomic(&m_ThreadState, ThreadState::RUNNING);

    m_Func(args);
}

} // namespace Hazel