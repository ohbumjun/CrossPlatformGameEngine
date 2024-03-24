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
        // thread 개수보다, length 가 적으면 n 은 0 이 나온다.
        // ex) Thread 개수는 4개인데, length 가 3개면 n 은 0 이 된다.
        threadCount = 1;
    }

    // threadCount 개수만큼의 ParallelData<T> 를 만든다.
    // 그리고 각각의 ParallelData<T> 는 threadCount 라는 변수로 초기화 된다.
    // ?? 2번째 threadCount 가 실질적인 역할을 하는가 ?
    std::vector<ParallelData<T>> params(threadCount, threadCount);

    for (size_t i = 0; i < threadCount; ++i)
    {
        uint64_t r = 0;
        Thread &t = *threadGroup[i];

        if (i == threadCount - 1)
        {
            // 마지막 thread 는 remain 을 r 로 한다.
            r = remain;
        }

        // 인자로 넘어온 userData 중에서 특정 범위의 데이터들만 실행한다.
        params[i].m_Range.start = n * i;
        params[i].m_Range.end = n + r;
        params[i].m_Data = userData;
        params[i].m_Action = &action;
        params[i].m_Thread = &t;
        // 각 Thread 를 다른 Core 에서 실행시키기 위해 Affinity 를 설정한다.
        params[i].m_Affinity = static_cast<uint8>(i + 1);

        t.SetPriority(ThreadPriority::HIGH);
    }

    for (size_t i = 0; i < threadCount; ++i)
    {
        Thread &t = *threadGroup[i];

        // 쓰레드 일을 실행한다.
        // [] 람다 함수 : 쓰레드가 할 일
        // *params[i]   : 할일에 대한 인자
        t.Start(
            [](void *args) {
                ParallelData<T> *p = (ParallelData<T> *)args;

                // 현재 해당 콜백을 실행하고 있는 Thread 정보를 가져온다.
                Thread *thread = p->thread;

                // 위에서 m_Affinity 는 static_cast<uint8>(i + 1) 값으로 세팅했다.
                // 즉, 각각 다른 core (혹은 core 내 하드웨어 쓰레드) 가 해당 일을
                // 실행하게 한다.
                thread->SetAffinity(p->affinity);

                // p->m_Scope, p->m_Data 라는 2개의 인자를 받는 함수를 호출한다.
                // 즉, 여러 쓰레드가, 같은 함수를 실행하되, 실행 인자들만 다르게 넘겨서 실행한다.
                ActionDelegate<Range, T>::Invoke(p->m_Action,
                                                 p->m_Range,
                                                 p->m_Data);
            },
            &params[i]);
    }

    // 모든 쓰레드 들이 일을 마칠 때까지 기다린다.
    // 현재 함수를 호출한 쓰레드는 모든 쓰레드가 일을 끝낼 때까지
    // 블로킹 상태에 놓이게 된다.
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
        // thread 개수보다, length 가 적으면 n 은 0 이 나온다.
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

        // action 은 callable object ex) lambda function, functor
        params[i].m_Action = &action;

        t.SetPriority(ThreadPriority::HIGH);
    }

    for (size_t i = 0; i < threadCount; ++i)
    {
        Thread &t = *threadGroup[i];

        t.Start(
            [](void *args) {
                ParallelData<T> *p = (ParallelData<T> *)args;

                // Affinity 설정 없이 호출한다.
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

// action : 람다함수 등 실제 호출할 함수
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
        // thread 개수보다, length 가 적으면 n 은 0 이 나온다.
        threadCount = 1;
    }

    // 이번에는 T 라는 Data Type 이 아니라
    // F* 라는 Type 에 대한 ParallelData 를 만든다.
    std::vector<ParallelData<F *>> params(threadCount, threadCount);

    for (size_t i = 0; i < threadCount; ++i)
    {
        uint64_t r = 0;
        Thread &t = *threadGroup[i];

        if (i == threadCount - 1)
            r = remain;

        params[i].m_Scope.start = n * i;
        params[i].m_Scope.end = params[i].m_Scope.start + n + r;
        // 해당 함수에서 m_Data 는 데이터가 아니라, 호출할 함수이다.
        params[i].m_Data = &action;

        t.SetPriority(ThreadPriority::HIGH);
    }

    for (size_t i = 0; i < threadCount; ++i)
    {
        Thread &t = *threadGroup[i];

        t.Start(
            [](void *args) {
                ParallelData<F *> *p = (ParallelData<F *> *)args;

                // p->m_Data 는 호출할 함수이다.
                // ->operator() 는 C++ 에서 callable object 를 호출하는 연산자이다.
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
    // 단 한번만 호출된다.
    // 함수 내의 local 변수가 static 으로 선언되면
    // 함수 호출 때마다 초기화 되는 것이 아니라, 최초 호출 때만 초기화 된다.
    static ThreadGroup threadGroups(threadCount);

    return threadGroups.threads;
}

} // namespace Hazel