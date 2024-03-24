
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

    // 1. Invoke �Լ��� �����ϴ� ������ "����"
    // 2. �ش� ������� �����ٷ��� ���� ������. ���� Thread::Invoke �Լ� ����
    // 3. Thread::Invoke -> Thread.m_Func (�ڽſ��� �Ҵ�� �ݹ��Լ�) ����
    ThreadUtils::RunThread(&m_Info, Invoke, this);
}

void Thread::Notify()
{
    // ���⿡ �� critical section lock �� �Ŵ� ����..?
    // 2�� �̻��� �����尡 ���� ���ÿ� ������, notify �� �Ϸ��� �Ѵ�.
    // �׷��� 2���� blocking �����带 running ���� ������ �ϴµ�
    // ����ȭ�� �ȵǼ� 1���� running ���� �������� �� �ֱ� �����̴�.

    // ��... �׷��� ���� �����忡�� �ش� Thread ��ü�� ������ �����Ѱ�..?
    // �ü�� ������ ~ Thread ��ü.�̷��� �ϳ��� �����Ǵ� �� �ƴѰ�..?
    ThreadUtils::LockCritSect(m_CricSect);
    m_IsWaiting = false;
    ThreadUtils::NotifyOneCondtion(m_Condition);
    ThreadUtils::UnlockCritSect(m_CricSect);
}

// �ش� m_Handle �� �����Ǵ� �����带 �����Ű�� �Լ�
void Thread::Stop()
{
    ThreadUtils::LockCritSect(m_CricSect);
    ThreadUtils::SetAtomic(&m_ThreadState, ThreadState::STOPPED);

    // Thread �� CloseHandle �Լ��� ȣ���Ѵ�.
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
        // �ش� condition variable �� signal �� ������ ��ٸ���.
        // ��, �ٸ� �����尡 �ش� m_Condition �� ���ؼ� notify, wake �� ������ ��ٸ���.
        // ?? : �׷��� ���� Thread ��ü�� ���̿� m_Condition �� �����Ǿ�� �ϴ°� �ƴѰ�..?
        //      �׷��� Thread ��ü �����ڿ��� ���� ������ ��������� ??
        ThreadUtils::WaitCondition(m_Condition,
                                   m_CricSect,
                                   THREAD_WAIT_INFINITE);

        // �ٷ� ������ ������ ��, ���� condition variable �� �������� ���� ��Ȳ�̶��,
        // ���ٿ��� ���� �����带 ���ŷ ���¿� ���̰� �ȴ�.
        // �׷��ٰ�, �ٸ� running thread �� ���� wake �ǰ�, ������ �����Ǹ�, �׶� ������ �����ϰ�
        // �� �������� �����ϰ� �Ǵ� �����̴�.
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
    // Thread ��ü�� ���� m_Handle �� �����Ǵ� thread �� ��� �ִ� �༮�� ���ε�...
    // ���� �ٸ� �����尡 �ش� Thread ��ü�� ���ؼ� GetThreadID �� ȣ���ϸ�...
    // ��Ȯ�� ������ ���� �� �ִ� ���ϱ� ?
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
    // @donghun �ƿ����� �ش� ������ ���ο��� ���� �ڽ��� �̸��� �ٲ� �� �ְ�, �ܺο��� �ٲ� ����� ���� ������ �ش� �Լ��� �ܺο� �����Ű�� �ʰ� �� �������� ó����
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