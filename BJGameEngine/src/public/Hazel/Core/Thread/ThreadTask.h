#pragma once

#include "hzpch.h"
namespace Hazel
{
// 할일. 에 대한 class
// - 기본적으로 할일 == callable object 로 정의된다.
template <class R>
class ThreadTask
{
};

// "함수 포인터" class template
template <class R, class... Args>
class ThreadTask<R(Args...)>
{
public:
    typedef R (*FuncPointer)(Args...);

    ThreadTask() : m_FuncPointer(nullptr)
    {
    }

    ThreadTask(ThreadTask &&other) noexcept : m_FuncPointer(other.m_FuncPointer)
    {
        other.m_FuncPointer = nullptr;
    }

    ThreadTask(const ThreadTask &task) : m_FuncPointer(task.m_FuncPointer)
    {
    }

    explicit ThreadTask(R (*function)(Args...)) : m_FuncPointer(function)
    {
    }

    virtual ~ThreadTask()
    {
    }

    virtual R RunTask(Args... args)
    {
        if (m_FuncPointer == nullptr)
            THROW("Callee is nullptr");

        return m_FuncPointer(args...);
    }

    FuncPointer m_FuncPointer;
};


// TClass 의 R 리턴 타입, Args 인자 목록을 가진, "멤버 함수". 에 대한 class template
// 즉, TClass 의 멤버함수에 대한 포인터. class template
template <class TClass, class R, class... Args>
class ThreadTask<R (TClass::*)(Args...)> : virtual public ThreadTask<R(Args...)>
{
public:
    typedef R (TClass::*MemberFuncPointer)(Args...);

    ThreadTask(TClass *instance)
        : m_Instance(m_ClassInstance), m_MemberFunc(nullptr),
          ThreadTask<R(Args...)>()

    {
    }

    ThreadTask(TClass *instance, ThreadTask &&other) noexcept
        : m_ClassInstance(instance), m_MemberFunc(other.m_MemberFunc),
          ThreadTask<R(Args...)>(other)
    {
        other.m_Instance = nullptr;
        other.m_MemberFunc = nullptr;
    }

    ThreadTask(TClass *instance, const ThreadTask &task)
        : m_ClassInstance(instance), m_MemberFunc(task.m_MemberFunc),
          ThreadTask<R(Args...)>(task)
    {
    }

    explicit ThreadTask(TClass *instance, R (TClass::*function)(Args...))
        : m_ClassInstance(instance), m_MemberFunc(function),
          ThreadTask<R(Args...)>()
    {
    }

    virtual ~ThreadTask()
    {
    }

    virtual R Run(Args... args)
    {
        if (m_ClassInstance == nullptr)
            THROW("instance is nullptr");

        if (m_MemberFunc == nullptr)
            THROW("calleeMember is nullptr");

        return (m_ClassInstance->*m_MemberFunc)(args...);
    }

    MemberFuncPointer m_MemberFunc;
    TClass *m_ClassInstance = nullptr;
};
} // namespace Hazel