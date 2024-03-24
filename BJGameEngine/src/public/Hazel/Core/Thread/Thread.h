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

    void Start(Func runnable = nullptr, void *userData = nullptr);

    template <typename Functor, typename... TArgs>
    void Start(Functor &&func, TArgs... args)
    {
        struct Param
        {
            Thread *thread;

            std::tuple<TArgs...> args;

            Functor functor;

            Param(Functor functor, Thread *thread, std::tuple<TArgs...> args)
                : functor(functor), args(args), thread(thread)
            {
            }
        };

        // Param 포인터를 인자로 받아서, 함수 포인터를 실행해주는 함수
        void (*Procedure)(void *) = [](void *p) {
            Param *parameters = static_cast<Param *>(p);

            HZ_CORE_ERROR("To Implement");
            // Invoker::Invoke(parameters->functor, parameters->args);

            delete parameters;
        };

        Param *param = new Param(func, this, std::make_tuple(args...));

        m_Func = Procedure;
        m_ArgData = param;

        // Thread 로 하여금 콜백함수를 실행하게 한다.
        ThreadUtils::RunThread(&m_Info, Invoke, this);
    }

    void Notify();

    void Stop();

    void Wait();

    // 해당 Thread 의 일을 끝내는 함수이다
    // ex) Worker::~Worker 함수에서 호출된다.
    void Join();

    inline ThreadState GetState()
    {
        return static_cast<ThreadState>(ThreadUtils::GetAtomic(&m_ThreadState));
    }

    inline void SetState(ThreadState state)
    {
        ThreadUtils::SetAtomic(&m_ThreadState, static_cast<int>(state));
    }

    inline ThreadInfo &GetThreadInfo()
    {
        return m_Info;
    }

    const char *GetName();

    /**
	 * @brief Thread 이름을 지정합니다. Start 하기 전에 지정하여야 하며 이후 지정할 수 없습니다.
	 */
    void SetName(char *name);

    /**
	 * @brief Thread 이름을 지정합니다. Start 하기 전에 지정하여야 하며 이후 지정할 수 없습니다.
	 */
    void SetName(const char *name);

    unsigned long GetThreadID();

    int GetPriority();

    void SetPriority(int priority);

    void SetAffinity(int affinity);

    static void Sleep(unsigned long milliseconds);

    inline CRIC_SECT *GetMutex()
    {
        return m_CricSect;
    }

    inline ConditionVar *GetCondition()
    {
        return m_Condition;
    }

private:
    Atomic m_ThreadState;

    ThreadInfo m_Info;

    // ThreadPool->Worker 에 의해 관리되는 Thread 들의 경우, 아래의 m_CricSect, m_Condition 변수는
    // ThreadPool 로부터 온 변수들이어야 하는 것 아닌가..?
    CRIC_SECT *m_CricSect;

    ConditionVar *m_Condition;

    // 해당 Thread 가 호출할 콜백함수
    Func m_Func;

    void *m_ArgData;

    int m_Priority;

    bool m_IsWaiting;

    static void *Invoke(void *args);

    virtual void Run(void *args);
};
} // namespace Hazel