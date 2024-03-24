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

        // Param �����͸� ���ڷ� �޾Ƽ�, �Լ� �����͸� �������ִ� �Լ�
        void (*Procedure)(void *) = [](void *p) {
            Param *parameters = static_cast<Param *>(p);

            HZ_CORE_ERROR("To Implement");
            // Invoker::Invoke(parameters->functor, parameters->args);

            delete parameters;
        };

        Param *param = new Param(func, this, std::make_tuple(args...));

        m_Func = Procedure;
        m_ArgData = param;

        // Thread �� �Ͽ��� �ݹ��Լ��� �����ϰ� �Ѵ�.
        ThreadUtils::RunThread(&m_Info, Invoke, this);
    }

    void Notify();

    void Stop();

    void Wait();

    // �ش� Thread �� ���� ������ �Լ��̴�
    // ex) Worker::~Worker �Լ����� ȣ��ȴ�.
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
	 * @brief Thread �̸��� �����մϴ�. Start �ϱ� ���� �����Ͽ��� �ϸ� ���� ������ �� �����ϴ�.
	 */
    void SetName(char *name);

    /**
	 * @brief Thread �̸��� �����մϴ�. Start �ϱ� ���� �����Ͽ��� �ϸ� ���� ������ �� �����ϴ�.
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

    // ThreadPool->Worker �� ���� �����Ǵ� Thread ���� ���, �Ʒ��� m_CricSect, m_Condition ������
    // ThreadPool �κ��� �� �������̾�� �ϴ� �� �ƴѰ�..?
    CRIC_SECT *m_CricSect;

    ConditionVar *m_Condition;

    // �ش� Thread �� ȣ���� �ݹ��Լ�
    Func m_Func;

    void *m_ArgData;

    int m_Priority;

    bool m_IsWaiting;

    static void *Invoke(void *args);

    virtual void Run(void *args);
};
} // namespace Hazel