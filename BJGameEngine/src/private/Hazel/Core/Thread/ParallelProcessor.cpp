#include "Hazel/Core/Thread/ParallelProcessor.h"
#include "Hazel/Core/Thread/ThreadUtil.h"

namespace Hazel
{
struct ThreadGroup
{
    std::vector<Thread *> threads;

    ThreadGroup(uint8 threadCount)
    {
        for (uint8 i = 0; i < threadCount; ++i)
        {
            threads.push_back(new Thread());
        }
    }

    ~ThreadGroup()
    {
        for (uint8 i = 0; i < threads.size(); ++i)
        {
            delete threads[i];
        }

        threads.clear();
    }
};

template <class T>
void ParallelProcessor::For(uint64_t index,
                            uint64_t length,
                            T userData,
                            ActionDelegate<Range, T> &action)
{
    if (0 == length)
        return;

    const size_t hardWareThreadCnt = ThreadUtils::GetThreadHardwareCount() - 1;
    std::vector<Thread *> &threadGroup = InitThreads(hardWareThreadCnt);
    size_t threadCount = threadGroup.size();

    // ex) length : 23 , threadCount : 4
    // -> n : 5, remain : 3
    uint64_t n = length / threadCount;
    uint64_t remain = length % threadCount;

    if (n == 0 && remain > 0)
    {
        // thread ��������, length �� ������ n �� 0 �� ���´�.
        // ex) Thread ������ 4���ε�, length �� 3���� n �� 0 �� �ȴ�.
        threadCount = 1;
    }

    // threadCount ������ŭ�� ParallelData<T> �� �����.
    // �׸��� ������ ParallelData<T> �� threadCount ��� ������ �ʱ�ȭ �ȴ�.
    // ?? 2��° threadCount �� �������� ������ �ϴ°� ?
    std::vector<ParallelData<T>> params(threadCount, threadCount);

    for (size_t i = 0; i < threadCount; ++i)
    {
        uint64_t r = 0;
        Thread &t = *threadGroup[i];

        if (i == threadCount - 1)
        {
            // ������ thread �� remain �� r �� �Ѵ�.
            r = remain;
        }

        // ���ڷ� �Ѿ�� userData �߿��� Ư�� ������ �����͵鸸 �����Ѵ�.
        params[i].m_Range.start = n * i;
        params[i].m_Range.end = n + r;
        params[i].m_Data = userData;
        params[i].m_Action = &action;
        params[i].m_Thread = &t;
        // �� Thread �� �ٸ� Core ���� �����Ű�� ���� Affinity �� �����Ѵ�.
        params[i].m_Affinity = static_cast<uint8>(i + 1);

        t.SetPriority(ThreadPriority::HIGH);
    }

    for (size_t i = 0; i < threadCount; ++i)
    {
        Thread &t = *threadGroup[i];

        // ������ ���� �����Ѵ�.
        // [] ���� �Լ� : �����尡 �� ��
        // *params[i]   : ���Ͽ� ���� ����
        t.Start(
            [](void *args) {
                ParallelData<T> *p = (ParallelData<T> *)args;

                // ���� �ش� �ݹ��� �����ϰ� �ִ� Thread ������ �����´�.
                Thread *thread = p->thread;

                // ������ m_Affinity �� static_cast<uint8>(i + 1) ������ �����ߴ�.
                // ��, ���� �ٸ� core (Ȥ�� core �� �ϵ���� ������) �� �ش� ����
                // �����ϰ� �Ѵ�.
                thread->SetAffinity(p->affinity);

                // p->m_Scope, p->m_Data ��� 2���� ���ڸ� �޴� �Լ��� ȣ���Ѵ�.
                // ��, ���� �����尡, ���� �Լ��� �����ϵ�, ���� ���ڵ鸸 �ٸ��� �Ѱܼ� �����Ѵ�.
                ActionDelegate<Range, T>::Invoke(p->m_Action,
                                                 p->m_Range,
                                                 p->m_Data);
            },
            &params[i]);
    }

    // ��� ������ ���� ���� ��ĥ ������ ��ٸ���.
    // ���� �Լ��� ȣ���� ������� ��� �����尡 ���� ���� ������
    // ���ŷ ���¿� ���̰� �ȴ�.
    for (size_t i = 0; i < threadGroup.size(); ++i)
    {
        threadGroup[i]->Join();
    }
}

template <typename T, typename F>
void ParallelProcessor::For(uint64_t index,
                            uint64_t length,
                            T userData,
                            F action)
{
    if (0 == length)
        return;

    const size_t hardWareThreadCnt =
        ThreadUtils::pr_thread_get_hardware_count() - 1;
    std::vector<Thread *> &threadGroup = InitThreads(hardWareThreadCnt);
    size_t threadCount = threadGroup.size();

    // ex) length : 23 , threadCount : 4
    // -> n : 5, remain : 3
    uint64_t n = length / threadCount;
    uint64_t remain = length % threadCount;

    if (n == 0 && remain > 0)
    {
        // thread ��������, length �� ������ n �� 0 �� ���´�.
        threadCount = 1;
    }

    std::vector<ParallelData<T>> params(threadCount, threadCount);

    for (size_t i = 0; i < threadCount; ++i)
    {
        uint64_t r = 0;
        Thread &t = *threadGroup[i];

        if (i == threadCount - 1)
        {
            r = remain;
        }

        params[i].m_Range.start = n * i;
        params[i].m_Range.end = n + r;
        params[i].m_Data = userData;

        // action �� callable object ex) lambda function, functor
        params[i].m_Action = &action;

        t.SetPriority(ThreadPriority::HIGH);
    }

    for (size_t i = 0; i < threadCount; ++i)
    {
        Thread &t = *threadGroup[i];

        t.Start(
            [](void *args) {
                ParallelData<T> *p = (ParallelData<T> *)args;

                // Affinity ���� ���� ȣ���Ѵ�.
                ActionDelegate<Range, T>::Invoke(p->m_Action,
                                                 p->m_Range,
                                                 p->m_Data);
            },
            &params[i]);
    }

    for (size_t i = 0; i < threadGroup.size(); ++i)
    {
        threadGroup[i]->Join();
    }
}

// action : �����Լ� �� ���� ȣ���� �Լ�
template <typename F>
void ParallelProcessor::For(uint64_t index, uint64_t length, F action)
{
    if (0 == length)
        return;

    const size_t hardWareThreadCnt =
        ThreadUtils::pr_thread_get_hardware_count() - 1;
    std::vector<Thread *> &threadGroup = InitThreads(hardWareThreadCnt);
    size_t threadCount = threadGroup.size();

    // ex) length : 23 , threadCount : 4
    // -> n : 5, remain : 3
    uint64_t n = length / threadCount;
    uint64_t remain = length % threadCount;

    if (n == 0 && remain > 0)
    {
        // thread ��������, length �� ������ n �� 0 �� ���´�.
        threadCount = 1;
    }

    // �̹����� T ��� Data Type �� �ƴ϶�
    // F* ��� Type �� ���� ParallelData �� �����.
    std::vector<ParallelData<F *>> params(threadCount, threadCount);

    for (size_t i = 0; i < threadCount; ++i)
    {
        uint64_t r = 0;
        Thread &t = *threadGroup[i];

        if (i == threadCount - 1)
            r = remain;

        params[i].m_Scope.start = n * i;
        params[i].m_Scope.end = params[i].m_Scope.start + n + r;
        // �ش� �Լ����� m_Data �� �����Ͱ� �ƴ϶�, ȣ���� �Լ��̴�.
        params[i].m_Data = &action;

        t.SetPriority(ThreadPriority::HIGH);
    }

    for (size_t i = 0; i < threadCount; ++i)
    {
        Thread &t = *threadGroup[i];

        t.Start(
            [](void *args) {
                ParallelData<F *> *p = (ParallelData<F *> *)args;

                // p->m_Data �� ȣ���� �Լ��̴�.
                // ->operator() �� C++ ���� callable object �� ȣ���ϴ� �������̴�.
                // ex) function pointer, functor, lambda function
                p->m_Data->operator()(p->m_Range);
            },
            &params[i]);
    }

    for (int i = 0; i < threadGroup.size(); ++i)
    {
        threadGroup[i]->Join();
    }
}


std::vector<Thread *> &ParallelProcessor::InitThreads(uint8 threadCount)
{
    // �� �ѹ��� ȣ��ȴ�.
    // �Լ� ���� local ������ static ���� ����Ǹ�
    // �Լ� ȣ�� ������ �ʱ�ȭ �Ǵ� ���� �ƴ϶�, ���� ȣ�� ���� �ʱ�ȭ �ȴ�.
    static ThreadGroup threadGroups(threadCount);

    return threadGroups.threads;
}

} // namespace Hazel