#pragma once
#include <functional>

#include "Hazel/Core/DataStructure/RingBuffer.h"

namespace Hazel
{
class Thread;
/**
 * 쓰레드 별 작업을 관리해주는 Class
 * ex) 사용 예시
* - ThreadExecuterManager::Main().Sync([]{  });
*                         다른 쓰레드에서, 메인쓰레드가 해당 일을 할 때까지 기다리게 하는 코드이다.
*                         즉, 메인 쓰레드가 아닌 다른 쓰레드에서, 메인쓰레드에게 일을 시키고
*                         메인 쓰레드가 일을 끝낼 때까지 블로킹 상태에 놓이게 하기 위한 코드인 것이다.            
* 					        따라서, 절대로 메인쓰레드에서 해당 코드를 부르면 안된다.
* - 절대 X : ThreadExecuter("SAMPLE")::Main().Async([]{  });
*                    Main()을 통해 메인 쓰레드에 예약
*                    SAMPLE은 대기중.
* 
* - ThreadExecuterManager("Hello").Async([]{});
*                         Hello이란 이름의 쓰레드에 예약
* 
* - ThreadExecuterManager::Main().Async([]{  });
*                         Main()을 통해 메인 쓰레드에 예약
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
	 * 해당 쓰레드에 작업을 등록하여 비동기로 완료하기 위한 함수이다.
	 */
        void AsyncExecute(std::function<void()> &&workCallback);

        /**
	 * 해당 쓰레드에 작업을 등록한 후 완료될 떄 까지 , 현재 함수를 호출한 쓰레드는
     * 블로킹 상태에 놓이게 한다.
     * 단, Main thread 에서 Main Dispatch Queue에 Sync를 걸면 교착 상태에 빠진다.
     * ex) 
	-> Main 입장에서는 condition_wait 을 함으로써, 실제 wait 을 하게 된다.
	-> 한편, 여기서 또 다른 Thread 를 새로 생성한다.

	해당 Thread 에서는
	DispatchQueue::runTask 함수 실행 
	- 만약 다른 쓰레드라면, 현재 main thread 는 잠들어있고
	해당 쓰레드만 실행되고 있는 상황
	- 그리고 DispatchQueue::runTask 는 자신이 들고 있는 _items , 즉
	  work 개수만큼 DispatchQueue::Handle::Execute() 를 실행한다.

	1) 해당 thread 가 실행할 일이 있을 때만 실행하도록
	while 문으로 spinLock

	2) DispatchQueue::Handle::Execute() 실행
	-> Main Thread 측에서 wait 한 condition variable 을 깨워준다.
	-> 이를 통해 다시 메인 쓰레드가 일할 수 있게 된다.
	-> 한편, 해당 쓰레드로 하여금 잠시 잠들게 만든다.
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
        * Thread 는, 아래의 함수를 통해 자기에게 할당 받은 일을 수행한다.
        * 그리고, 아래 함수 안에서는, 바로 위의 Task._task callback 함수를 실행학 ㅔ된다.
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
	 *메인 쓰레드용 핸들을 세팅해준다. 단, 어플리케이션 실행 중 한번만 호출할 수 있다.
	 */
    static ThreadHandle *Initialize();
    static void Finalize();

    /**
	 * 메인 쓰레드에서 실행하는 Thread Executer 이다.
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

