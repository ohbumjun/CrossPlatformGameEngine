#pragma once
#include "ThreadUtil.h"
#include "ThreadPool.h"
#include "Hazel/Core/Reflection/TypeId.h"
#include "Hazel/Core/Reflection/Reflection.h"
#include "Jobs.h"
// #include "../Allocator/EngineAllocator.h"
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

    static void Initialize();
    static void Finalize();
    static void UnregistAll();
    template <class T,
              typename std::enable_if<
                  std::is_base_of<JobContainer, T>::value>::type * = nullptr>
    static T *MakeThreadJob();

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
		* EngineJob 객체들의 실행 메소드
		* 
		* (세부사항)
		* JobContainer 안에 있는 ActiveJob 하나를 JobManager 의
		* ThreadPool 에게 시킨다.
	*/
    static void ExecuteWithoutCtx(JobContainer *job);

    /**
		* @brief LvEngineJob을 jobCount, groupCount를 이용해 Job을 나눠서 Parallel하게 실행 메소드
			- 주의 : Dispatch 실행시 Job수가 증가하여 Wait(size_t taskCount)로 대기시 무한대기, 데이타 레이스 발생가능
		- jobContainer	: Thread 들에게 할당할 ActiveJob 들을 가지고 있는 Container
		-  totalJobCount : 실행할 전체 Job 수
		* @param divideCount 하나의 Job Group별 최대 갯수
	*/
    static void ExecuteParallel(JobContext &ctx,
                                JobContainer *jobContainer,
                                size_t totalJobCount,
                                size_t divideCount);
    /**
		* ExecuteParallel와 동일. 단, Wait 조건이 Ctx 가 아니라, ThreadPool 의 cnt
	*/
    static void ExecuteParallelNoCtx(JobContainer *job,
                                     size_t jobCount,
                                     size_t divideCount);

    /**
		* Job 실행을 기다리는 함수
		* taskCount : 기다릴 Job의 수
	*/
    static void Wait(JobContext &ctx, int32 taskCount);
    /**
		* 위 함수와 동일, 단 Thread Pool 의 finishcnt 가 taskCount 될때까지
	*/
    static void WaitNoCtx(int32 taskCount);
    /**
		* ttx total cnt 만큼의 job 이 끝날 때까지 기다리기 
	*/
    static void WaitAll(JobContext &ctx);
    /**
		* @brief ThreadPool 내의 모든 Job이 끝날 때까지 기다리기
	*/
    static void WaitAllNoCtx();

private:
    // 같은 종류의 JobContainer 들을 모아둔 Class
    class JobContainerGroup
    {
    public:
        JobContainerGroup(TypeId jobType) : m_JobIncreaseCount(5)
        {
            m_Type = jobType;

            // if (false == LvReflection::HasRegist(_type))
            // {
            // 	LV_LOG(crash, "JobPool LvTypeId jobType is not regist type");
            // }

            ThreadUtils::InitSpinLock(&m_SpinLock);
        }

        virtual ~JobContainerGroup()
        {
            size_t size = Reflection::GetTypeSize(m_Type);

            char *buff = nullptr;

            // 미리 제작한 job Free
            for (JobContainer *jobContainer : m_JobContainers)
            {
                // buff = reinterpret_cast<char*>(job);
                // for (uint32 i = 0; i < m_JobIncreaseCount; ++i)
                // {
                // 	// Call Constructor
                // 	// LvReflection::Deconstruct(m_Type, buff + (size * i));
                // }
                jobContainer->~JobContainer();

                // EngineAllocator::Free(job);
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
            ThreadUtils::LockSpinLock(&m_GroupLock);

            //찾기 - 임계영역 _allocatedJobsArray
            for (JobContainer *engineJob : m_Jobs)
            {
                if (isJobWorkPossible(engineJob))
                {
                    ThreadUtils::UnlockSpinLock(&m_SpinLock);

                    return (T *)engineJob;
                }
            }

            ThreadUtils::UnlockSpinLock(&m_GroupLock);

            return nullptr;
        }

        template <class T>
        void increaseJobContainer()
        {
            ThreadUtils::InitSpinLock(&m_GroupLock);

            //사용가능한 Task 미리 제작 임계영역 _allocatedJobsArray
            for (uint32 i = 0; i < m_JobIncreaseCount; ++i)
            {
                T *jobMemory = (T *)EngineAllocator::Allocate(
                    DataType::RAWDATA,
                    sizeof(T),
                    Reflection::GetTypeName<T>());

                //Call Constructor
                T *newJob = new (jobMemory) T();

                // LvReflection::Construct(_type, &jobSources[i]);

                m_JobContainers.push_back(newJob);
            }

            ThreadUtils::UnlockSpinLock(&m_SpinLock);
        }

        inline bool isJobWorkPossible(JobContainer *job)
        {
            return job->setWorkState();
        }

        // 늘릴 때 해당 데이터에 접근 불가능하게 해야함
        std::vector<JobContainer *> m_JobContainers;

        // 각 JobContainerGroup 마다, 동기화를 진행해주고자 한다.
        SpinLock m_GroupLock;
        const uint32 m_JobIncreaseCount;
        TypeId m_Type = 0;
    };

    static ThreadPool *m_ThreadPool;
    static SpinLock m_SpinLock;
    static std::unordered_map<TypeId /*Pool ID*/, JobContainerGroup>
        m_JobContainerGroups;
};

} // namespace Hazel