#pragma once

#include "ThreadPool.h"


// https://www.codeproject.com/Articles/1170503/The-Impossibly-Fast-Cplusplus-Delegates-Fixed
namespace Hazel
{
/*
* 위의 내용에 근거하면, 아래의 AcitionDelegate 를 이용하여 구현한 callback 로직이
* std::function 보다 빠르다고 한다.
*/
template <class... Args>
class ActionDelegate
{
public:
    typedef void (*Delegate)(Args...);

    Delegate m_Action;

    explicit ActionDelegate(void (*action)(Args...)) : m_Action(action)
    {
    }

    static void Invoke(ActionDelegate *action, Args... args)
    {
        action->m_Action(args...);
    }

    void operator()(const ActionDelegate &action, Args... args)
    {
        action->m_Action(args...);
    }
};

// Parallel Processing Task 를 위한 Class
class ParallelProcessor
{
public:
    struct Range
    {
        size_t start;
        size_t end;

        Range() : start(0), end(0)
        {
        }
    };

    // 특정 범위에 대한 parallel 작업을 진행시키는 함수들
    // 1) ActionDelegate 를 이용한 함수
    template <class T>
    static void For(uint64_t index,
                    uint64_t length, /*넘겨준 데이터의 개수*/
                    T userData,
                    ActionDelegate<Range, T> &action);

    // 2) F : callable object (ex. lambda function, functor, function pointer)
    // ex) std::function 말고, 주로 lambda 함수를 인자로 받는 것 처럼 보인다.
    // 또한 1) 과의 차이는 Affinity 를 설정하지 않는 것도 있다.
    template <typename T, typename F>
    static void For(uint64_t index, uint64_t length, T userData, F action);

    // 3) 여기서는 Data 없이, 호출할 함수만을 넘겨준다.
    template <typename F>
    static void For(uint64_t index, uint64_t length, F action);

private:
    template <class T>
    struct ParallelData
    {
        ParallelData() : m_Action(nullptr)
        {
        }

        ParallelData(Range range, T data, ActionDelegate<Range, T> *action)
            : m_Range(range), m_Data(data), m_Action(action)
        {
        }

        Range m_Range; // 처리할 범위
        T m_Data;      // 처리할 Data

        //" Range와 T 를 인자로 받는 함수"를 실행하는 Delegate
        ActionDelegate<Range, T> *m_Action;
        Thread *
            m_Thread; // 처리를 담당하는 Thread : ParallelProcessor 가 들고 있는 Thread 중 하나
        uint8 m_Affinity;
    };

    static std::vector<Thread *> &InitThreads(uint8 threadCount);
};

} // namespace Hazel