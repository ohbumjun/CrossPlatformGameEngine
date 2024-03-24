#pragma once

#include "ThreadPool.h"


// https://www.codeproject.com/Articles/1170503/The-Impossibly-Fast-Cplusplus-Delegates-Fixed
namespace Hazel
{
/*
* ���� ���뿡 �ٰ��ϸ�, �Ʒ��� AcitionDelegate �� �̿��Ͽ� ������ callback ������
* std::function ���� �����ٰ� �Ѵ�.
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

// Parallel Processing Task �� ���� Class
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

    // Ư�� ������ ���� parallel �۾��� �����Ű�� �Լ���
    // 1) ActionDelegate �� �̿��� �Լ�
    template <class T>
    static void For(uint64_t index,
                    uint64_t length, /*�Ѱ��� �������� ����*/
                    T userData,
                    ActionDelegate<Range, T> &action);

    // 2) F : callable object (ex. lambda function, functor, function pointer)
    // ex) std::function ����, �ַ� lambda �Լ��� ���ڷ� �޴� �� ó�� ���δ�.
    // ���� 1) ���� ���̴� Affinity �� �������� �ʴ� �͵� �ִ�.
    template <typename T, typename F>
    static void For(uint64_t index, uint64_t length, T userData, F action);

    // 3) ���⼭�� Data ����, ȣ���� �Լ����� �Ѱ��ش�.
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

        Range m_Range; // ó���� ����
        T m_Data;      // ó���� Data

        //" Range�� T �� ���ڷ� �޴� �Լ�"�� �����ϴ� Delegate
        ActionDelegate<Range, T> *m_Action;
        Thread *
            m_Thread; // ó���� ����ϴ� Thread : ParallelProcessor �� ��� �ִ� Thread �� �ϳ�
        uint8 m_Affinity;
    };

    static std::vector<Thread *> &InitThreads(uint8 threadCount);
};

} // namespace Hazel