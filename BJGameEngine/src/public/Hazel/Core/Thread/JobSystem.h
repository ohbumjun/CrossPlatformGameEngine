#pragma once
#include "ThreadUtil.h"
#include "ThreadPool.h"
#include "Hazel/Core/Reflection/TypeId.h"
#include "Hazel/Core/Reflection/Reflection.h"
#include "Jobs.h"

namespace Hazel
{
class JobContext
{
    friend class JobManager;
    friend class JobContainer;

    Atomic m_FinishTaskCount;
    Atomic m_TotalTaskCount;

public:
    JobContext()
    {
        ThreadUtils::SetAtomic(&m_FinishTaskCount, 0);
        ThreadUtils::SetAtomic(&m_TotalTaskCount, 0);
    }
};

// 모든 종류의 Job 을 관리하는 총 관리자 Class
// 1) ThreadPool
// 2) Job Group
// 을 들고 있으면서 여러 쓰레드로 여러 Job 을 수행하는 기능을 담당한다.
class JobManager
{
public:
    friend class JobContext;

    template <class T,
              typename std::enable_if<
                  std::is_base_of<JobContainer, T>::value>::type * = nullptr>
    static T *MakeThreadJob();
    static void Finalize();
    static void Initialize();

    /**
		* EngineJob 객체들의 실행 메소드
		* 
		* (세부사항)
		* JobContainer 안에 있는 ActiveJob 하나를 JobManager 의
		* ThreadPool 에게 시킨다.
	*/
    static void ExecuteWithoutCtx(JobContainer *job);
    /**
		* EngineJob 의 실행 메소드
		* ctx : job 을 실행하는 맥락 ex) total cnt, complete cnt 등
		* jobContainer : 실행할 job을 가지고 있는 container
		* 
		* (세부사항)
		* JobContainer 안에 있는 ActiveJob 하나를 JobManager 의
		* ThreadPool 에게 시킨다.
	*/
    static void Execute(JobContext &ctx, JobContainer *jobContainer);

    /**
		* ExecuteParallel와 동일. 단, Wait 조건이 Ctx 가 아니라, ThreadPool 의 cnt
	*/
    static void ExecuteParallelNoCtx(JobContainer *job,
                                     size_t jobCount,
                                     size_t divideCount);
    /**
		*LvEngineJob을 jobCount, groupCount를 이용해 Job을 나눠서 Parallel하게 실행 메소드
			- 주의 : Dispatch 실행시 Job수가 증가하여 Wait(size_t taskCount)로 대기시 무한대기, 데이타 레이스 발생가능
		- jobContainer	: Thread 들에게 할당할 ActiveJob 들을 가지고 있는 Container
		-  totalJobCount : 실행할 전체 Job 수
		* @param divideCount 하나의 Job Group별 최대 갯수
	*/
    static void ExecuteParallel(JobContext &jobContext,
                                JobContainer *jobContainer,
                                size_t totalJobCount,
                                size_t divideCount);
   
    /**
		* 위 함수와 동일, 단 Thread Pool 의 finishcnt 가 taskCount 될때까지
	*/
    static void WaitNoCtx(int32 taskCount);
    /**
		* @brief ThreadPool 내의 모든 Job이 끝날 때까지 기다리기
	*/
    static void WaitAllNoCtx();

    /**
		* Job 실행을 기다리는 함수
		* taskCount : 기다릴 Job의 수
	*/
    static void Wait(JobContext &ctx, int32 taskCount);
    /**
		* ttx total cnt 만큼의 job 이 끝날 때까지 기다리기 
	*/
    static void WaitAll(JobContext &ctx);
    

private:
    // 같은 종류의 JobContainer 들을 모아둔 Class
    class JobContainerGroup
    {
    public:
        JobContainerGroup(TypeId jobType) : m_JobIncreaseCount(5)
        {
            m_Type = jobType;
        }

        virtual ~JobContainerGroup()
        {
            size_t size = Reflection::GetTypeSize(m_Type);

            for (JobContainer *jobContainer : m_JobContainers)
            {
                jobContainer->~JobContainer();

                free(jobContainer);
            }
        }

        // 여기서 T 는 Job Class Type 이 될 것이다.
        // ex) SortJob
        template <class T>
        T *GetRestJobContainer()
        {
            //임계영역 각 과정내에 lock 존재
            //무조건 return 함
            while (true)
            {
                if (T *joContainerb = findRestJobContainer<T>())
                {
                    return job;
                }

                increaseJobContainer<T>(); //이거 맘에안듬
            }
        }

        static std::string TypeName()
        {
            return "LvEngineJobSystem::JobPool";
        }

        const std::vector<JobContainer *> &getJobContainers()
        {
            return m_JobContainers;
        }

    private:
        template <class T>
        T *findRestJobContainer()
        {
            //찾기 - 임계영역 _allocatedJobsArray
            for (JobContainer *engineJob : m_Jobs)
            {
                if (isJobWorkPossible(engineJob))
                {
                    ThreadUtils::UnlockSpinLock(&m_SpinLock);

                    return (T *)engineJob;
                }
            }
            return nullptr;
        }

        template <class T>
        void increaseJobContainer()
        {
            //사용가능한 Task 미리 제작 임계영역 _allocatedJobsArray
            for (uint32 i = 0; i < m_JobIncreaseCount; ++i)
            {
                T *jobMemory = (T *)EngineAllocator::Allocate(
                    DataType::RAWDATA,
                    sizeof(T),
                    Reflection::GetTypeName<T>());

                T *newJob = new (jobMemory) T();

                m_JobContainers.push_back(newJob);
            }
        }

        inline bool isJobWorkPossible(JobContainer *job)
        {
            return job->setToRunState();
        }

        // 늘릴 때 해당 데이터에 접근 불가능하게 해야함
        std::vector<JobContainer *> m_JobContainers;

        const uint32 m_JobIncreaseCount;
        TypeId m_Type = 0;
    };

    static ThreadPool *m_ThreadPool;
    static std::unordered_map<TypeId /*Pool ID*/, JobContainerGroup>
        m_JobContainerGroups;
};

} // namespace Hazel