#pragma once

#include "ThreadUtil.h"
#include "Hazel/Utils/Log.h"

namespace Hazel
{
class Thread
{
    typedef void (*Func)(void *);

public:
    Thread();

    explicit Thread(Func procedure, void *userData);

    ~Thread();

    void StartThread(Func procedure = nullptr, void *userData = nullptr);

    template <typename Functor, typename... TArgs>
    void StartThread(Functor &&func, TArgs... args)
    {
        struct FuncParam
        {
            Thread *thread;

            std::tuple<TArgs...> args;

            Functor functor;

            FuncParam(Functor functor,
                      Thread *thread,
                      std::tuple<TArgs...> args)
                : functor(functor), args(args), thread(thread)
            {
            }
        };

        // Param �����͸� ���ڷ� �޾Ƽ�, �Լ� �����͸� �������ִ� �Լ�
        void (*Procedure)(void *) = [](void *p) {
            FuncParam *parameters = static_cast<FuncParam *>(p);

            HZ_CORE_ERROR("To Implement");
            // Invoker::Invoke(parameters->functor, parameters->args);

            delete parameters;
        };

        FuncParam *param = new FuncParam(func, this, std::make_tuple(args...));

        m_Func = Procedure;
        m_ArgData = param;

        // Thread �� �Ͽ��� �ݹ��Լ��� �����ϰ� �Ѵ�.
        ThreadUtils::RunThread(&m_Info, Invoke, this);
    }

    void NotifyThread();

    void StopThread();

    void WaitThread();

    // �ش� Thread �� ���� ������ �Լ��̴�
    // ex) Worker::~Worker �Լ����� ȣ��ȴ�.
    void JoinThread();

    inline ThreadState GetThreadState()
    {
        return static_cast<ThreadState>(ThreadUtils::GetAtomic(&m_ThreadState));
    }

    inline void SetThreadState(ThreadState state)
    {
        ThreadUtils::SetAtomic(&m_ThreadState, static_cast<int>(state));
    }

    inline ThreadInfo &GetThreadInfo()
    {
        return m_Info;
    }

    const char *GetThreadName();
   
    void SetThreadName(char *name);

    void SetThreadName(const char *name);

    unsigned long GetThreadID();

    int GetPriority();

    void SetPriority(int priority);

    void SetAffinity(int affinity);

    static void SleepThread(unsigned long milliseconds);

    inline CRIC_SECT *GetMutex()
    {
        return m_CricSect;
    }

    inline ConditionVariable *GetCondition()
    {
        return m_Condition;
    }

private:
    Atomic m_ThreadState;

    ThreadInfo m_Info;

    CRIC_SECT *m_CricSect;

    ConditionVariable *m_Condition;

    // �ش� Thread �� ȣ���� �ݹ��Լ�
    Func m_Func;

    void *m_ArgData;

    int m_Priority;

    bool m_IsWaiting;

    static void *Invoke(void *args);

    virtual void RunThread(void *args);
};
} // namespace Hazel