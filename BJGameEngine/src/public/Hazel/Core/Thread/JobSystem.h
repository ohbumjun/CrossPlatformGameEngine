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

// ��� ������ Job �� �����ϴ� �� ������ Class
// 1) ThreadPool
// 2) Job Group
// �� ��� �����鼭 ���� ������� ���� Job �� �����ϴ� ����� ����Ѵ�.
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
		* EngineJob �� ���� �޼ҵ�
		* ctx : job �� �����ϴ� �ƶ� ex) total cnt, complete cnt ��
		* jobContainer : ������ job�� ������ �ִ� container
		* 
		* (���λ���)
		* JobContainer �ȿ� �ִ� ActiveJob �ϳ��� JobManager ��
		* ThreadPool ���� ��Ų��.
	*/
    static void Execute(JobContext &ctx, JobContainer *jobContainer);

    /**
		* EngineJob ��ü���� ���� �޼ҵ�
		* 
		* (���λ���)
		* JobContainer �ȿ� �ִ� ActiveJob �ϳ��� JobManager ��
		* ThreadPool ���� ��Ų��.
	*/
    static void ExecuteWithoutCtx(JobContainer *job);

    /**
		* @brief LvEngineJob�� jobCount, groupCount�� �̿��� Job�� ������ Parallel�ϰ� ���� �޼ҵ�
			- ���� : Dispatch ����� Job���� �����Ͽ� Wait(size_t taskCount)�� ���� ���Ѵ��, ����Ÿ ���̽� �߻�����
		- jobContainer	: Thread �鿡�� �Ҵ��� ActiveJob ���� ������ �ִ� Container
		-  totalJobCount : ������ ��ü Job ��
		* @param divideCount �ϳ��� Job Group�� �ִ� ����
	*/
    static void ExecuteParallel(JobContext &ctx,
                                JobContainer *jobContainer,
                                size_t totalJobCount,
                                size_t divideCount);
    /**
		* ExecuteParallel�� ����. ��, Wait ������ Ctx �� �ƴ϶�, ThreadPool �� cnt
	*/
    static void ExecuteParallelNoCtx(JobContainer *job,
                                     size_t jobCount,
                                     size_t divideCount);

    /**
		* Job ������ ��ٸ��� �Լ�
		* taskCount : ��ٸ� Job�� ��
	*/
    static void Wait(JobContext &ctx, int32 taskCount);
    /**
		* �� �Լ��� ����, �� Thread Pool �� finishcnt �� taskCount �ɶ�����
	*/
    static void WaitNoCtx(int32 taskCount);
    /**
		* ttx total cnt ��ŭ�� job �� ���� ������ ��ٸ��� 
	*/
    static void WaitAll(JobContext &ctx);
    /**
		* @brief ThreadPool ���� ��� Job�� ���� ������ ��ٸ���
	*/
    static void WaitAllNoCtx();

private:
    // ���� ������ JobContainer ���� ��Ƶ� Class
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

            // �̸� ������ job Free
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

        // ���⼭ T �� Job Class Type �� �� ���̴�.
        // ex) SortJob
        template <class T>
        T *GetRestJobContainer()
        {
            //�Ӱ迵�� �� �������� lock ����
            //������ return ��
            while (true)
            {
                if (T *joContainerb = findRestJobContainer<T>())
                {
                    return job;
                }

                increaseJobContainer<T>(); //�̰� �����ȵ�
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

            //ã�� - �Ӱ迵�� _allocatedJobsArray
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

            //��밡���� Task �̸� ���� �Ӱ迵�� _allocatedJobsArray
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

        // �ø� �� �ش� �����Ϳ� ���� �Ұ����ϰ� �ؾ���
        std::vector<JobContainer *> m_JobContainers;

        // �� JobContainerGroup ����, ����ȭ�� �������ְ��� �Ѵ�.
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