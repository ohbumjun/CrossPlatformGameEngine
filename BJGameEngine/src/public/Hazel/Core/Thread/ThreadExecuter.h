#pragma once
#include <functional>

#include "Hazel/Core/DataStructure/RingBuffer.h"

namespace Hazel
{
class Thread;
/**
 * ������ �� �۾��� �������ִ� Class
 * ex) ��� ����
* - ThreadExecuterManager::Main().Sync([]{  });
*                         �ٸ� �����忡��, ���ξ����尡 �ش� ���� �� ������ ��ٸ��� �ϴ� �ڵ��̴�.
*                         ��, ���� �����尡 �ƴ� �ٸ� �����忡��, ���ξ����忡�� ���� ��Ű��
*                         ���� �����尡 ���� ���� ������ ���ŷ ���¿� ���̰� �ϱ� ���� �ڵ��� ���̴�.            
* 					        ����, ����� ���ξ����忡�� �ش� �ڵ带 �θ��� �ȵȴ�.
* - ���� X : ThreadExecuter("SAMPLE")::Main().Async([]{  });
*                    Main()�� ���� ���� �����忡 ����
*                    SAMPLE�� �����.
* 
* - ThreadExecuterManager("Hello").Async([]{});
*                         Hello�̶� �̸��� �����忡 ����
* 
* - ThreadExecuterManager::Main().Async([]{  });
*                         Main()�� ���� ���� �����忡 ����
* 
 */
class ThreadExecuterManager
{

    struct ExecuterTask
    {
        ExecuterTask(std::function<void()> &&task) : _task(std::move(task))
        {
        }

        ExecuterTask(const std::function<void()> &task) : _task(task)
        {
        }

        std::function<void()> _task;
    };

public:
    class ThreadHandle;
    class ThreadExecuter
    {
        friend class ThreadExecuterManager;
        ~ThreadExecuter();

    public:
        /**
	 * �ش� �����忡 �۾��� ����Ͽ� �񵿱�� �Ϸ��ϱ� ���� �Լ��̴�.
	 */
        void AsyncExecute(std::function<void()> &&workCallback);

        /**
	 * �ش� �����忡 �۾��� ����� �� �Ϸ�� �� ���� , ���� �Լ��� ȣ���� �������
     * ���ŷ ���¿� ���̰� �Ѵ�.
     * ��, Main thread ���� Main Dispatch Queue�� Sync�� �ɸ� ���� ���¿� ������.
     * ex) 
	-> Main ���忡���� condition_wait �� �����ν�, ���� wait �� �ϰ� �ȴ�.
	-> ����, ���⼭ �� �ٸ� Thread �� ���� �����Ѵ�.

	�ش� Thread ������
	DispatchQueue::runTask �Լ� ���� 
	- ���� �ٸ� ��������, ���� main thread �� �����ְ�
	�ش� �����常 ����ǰ� �ִ� ��Ȳ
	- �׸��� DispatchQueue::runTask �� �ڽ��� ��� �ִ� _items , ��
	  work ������ŭ DispatchQueue::Handle::Execute() �� �����Ѵ�.

	1) �ش� thread �� ������ ���� ���� ���� �����ϵ���
	while ������ spinLock

	2) DispatchQueue::Handle::Execute() ����
	-> Main Thread ������ wait �� condition variable �� �����ش�.
	-> �̸� ���� �ٽ� ���� �����尡 ���� �� �ְ� �ȴ�.
	-> ����, �ش� ������� �Ͽ��� ��� ���� �����.
	*/
        void SyncExecute(std::function<void()> &&workCallback);
        
    private:
        void asyncExecute(ExecuterTask *workItem) const;

        uint64 m_ThreadId;
        std::string m_Desc;
        ThreadHandle *m_Handle;
        RingBuffer<ExecuterTask *> *m_Tasks;
        Thread * m_Thread;
        CRIC_SECT * m_Mutex;
        ConditionVariable *m_Condition;
    };

    class ThreadHandle
    {
    public:
        /*
        * Thread ��, �Ʒ��� �Լ��� ���� �ڱ⿡�� �Ҵ� ���� ���� �����Ѵ�.
        * �׸���, �Ʒ� �Լ� �ȿ�����, �ٷ� ���� Task._task callback �Լ��� ������ �ĵȴ�.
        */
        bool ExecuteHandle();

        ThreadHandle(const uint64 id) : m_SelfId(id)
        {
        }
        ThreadHandle(ThreadExecuter &target) : m_SelfId(target.m_ThreadId)
        {
        }
        ~ThreadHandle();

    private:
        const uint64 m_SelfId;
        ExecuterTask *m_CurrentItem = nullptr;
    };

    /**
	 *���� ������� �ڵ��� �������ش�. ��, ���ø����̼� ���� �� �ѹ��� ȣ���� �� �ִ�.
	 */
    static ThreadHandle *Initialize();
    static void Finalize();

    /**
	 * ���� �����忡�� �����ϴ� Thread Executer �̴�.
	 */
    static ThreadExecuter &GetMain()
    {
        return *_threadExecuters[m_MainExecuterId];
    }

    static uint64 CreateExecuter(const char *description);

    ThreadExecuterManager(const char *description = "");

private:

    static std::unordered_map<uint64, ThreadExecuter *> _threadExecuters;
    static std::vector<uint64> _threadIds;
    
    static RingBuffer<ExecuterTask *> *getExecuterTasks(uint64 id);
    static void releaseExecuter(uint64 id);
    static const char *getExecuterDesc(uint64 id);
    static void runExecuterTask(void *argThreadId);

    ThreadExecuterManager(uint64 id);


    static ThreadHandle *m_MainThreadHandle;
    static uint64 m_MainExecuterId;

};


}

