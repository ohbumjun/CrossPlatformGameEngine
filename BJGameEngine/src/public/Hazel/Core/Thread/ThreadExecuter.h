#pragma once
#include <functional>

#include "Hazel/Core/DataStructure/RingBuffer.h"

namespace Hazel
{
class Thread;
/**
 * ������ �� �۾��� �������ִ� Class
 * ex) ��� ����
* - ThreadExecuter("Hello").Async([]{});
*                         Hello�̶� �̸��� �����忡 ����
* 
* - ThreadExecuter::Main().Async([]{  });
*                         Main()�� ���� ���� �����忡 ����
* 
* - ThreadExecuter::Main().Sync([]{  });
*                         �ٸ� �����忡��, ���ξ����尡 �ش� ���� �� ������ ��ٸ��� �ϴ� �ڵ��̴�.
*                         ��, ���� �����尡 �ƴ� �ٸ� �����忡��, ���ξ����忡�� ���� ��Ű��
*                         ���� �����尡 ���� ���� ������ ���ŷ ���¿� ���̰� �ϱ� ���� �ڵ��� ���̴�.            
* 					        ����, ����� ���ξ����忡�� �ش� �ڵ带 �θ��� �ȵȴ�.
* - ���� X : ThreadExecuter("SAMPLE")::Main().Async([]{  });
*                    Main()�� ���� ���� �����忡 ����
*                    SAMPLE�� �����.
 */
class ThreadExecuter
{
    struct Task
    {
        Task(std::function<void()> &&task) : _task(std::move(task))
        {
        }

        Task(const std::function<void()> &task) : _task(task)
        {
        }

        std::function<void()> _task;
    };

public:
    class ThreadHandle
    {
    public:
        /*
        * Thread ��, �Ʒ��� �Լ��� ���� �ڱ⿡�� �Ҵ� ���� ���� �����Ѵ�.
        * �׸���, �Ʒ� �Լ� �ȿ�����, �ٷ� ���� Task._task callback �Լ��� ������ �ĵȴ�.
        */
        bool ExecuteHandle();

        ThreadHandle(const uint64 id) : _selfId(id)
        {
        }
        ThreadHandle(ThreadExecuter &target) : _selfId(target._threadId)
        {
        }
        ~ThreadHandle();

    private:
        const uint64 _selfId;
        Task *_currentItem = nullptr;
    };


public:
    /**
	 *���� ������� �ڵ��� �������ش�. ��, ���ø����̼� ���� �� �ѹ��� ȣ���� �� �ִ�.
	 */
    static ThreadHandle *Init();
    static void Finalize();

    /**
	 * ���� �����忡�� �����ϴ� Thread Executer �̴�.
	 */
    static ThreadExecuter &Main()
    {
        return *_mainExecuter;
    }

    ThreadExecuter(const char *description = "");

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

    struct Desc
    {
        std::string desc;
    };

    static std::vector<uint64> _threadIds;
    static std::unordered_map<uint64, ThreadHandle *> _threadHandles;
    static std::unordered_map<uint64 /*Thread Handle*/, RingBuffer<Task *> *>
        _threadTasks;
    static std::unordered_map<uint64, Desc> _threadDescriptions;
    static std::unordered_map<uint64, Thread *> _threads;
    static std::unordered_map<uint64, CRIC_SECT *> _threadMutexes;
    static std::unordered_map<uint64, ConditionVar *> _threadConditions;

    static uint64 createThread(const char *description);
    static void releaseThread(uint64 id);
    static RingBuffer<Task *> *getThreadTasks(uint64 id);
    static const char *getThreadDesc(uint64 id);
    static void runThreadTask(void *argThreadId);

    ThreadExecuter(uint64 id);

    void async(Task *workItem) const;

    const uint64 _threadId;
    const char *_description;

    static ThreadHandle *_mainThreadHandle;
    static ThreadExecuter *_mainExecuter;

};


}

