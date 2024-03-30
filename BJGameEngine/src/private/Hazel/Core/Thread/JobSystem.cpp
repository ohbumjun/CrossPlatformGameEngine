#include "Hazel/Core/Thread/JobSystem.h"
// #include "../Allocator/EngineAllocator.h"
#include "Hazel/Core/Reflection/TypeId.h"
#include "Hazel/Core/Thread/ThreadUtil.h"
#include "Hazel/Core/Thread/ParallelProcessor.h"

namespace Hazel
{
ThreadPool *JobManager::m_ThreadPool;
SpinLock JobManager::m_SpinLock;
std::unordered_map<TypeId /*Pool ID*/, JobManager::JobContainerGroup>
    JobManager::m_JobContainerGroups;


void JobManager::Finalize()
{
    // ThreadPool 내의 Thread 들이 모두 종료될 때까지 기다린다.
    WaitAllNoCtx();

    m_ThreadPool->~ThreadPool();

    free(m_ThreadPool);

    for (auto &jobGroupTable : m_JobContainerGroups)
    {
        JobContainerGroup &jobGroup = jobGroupTable.second;

        jobGroup.~JobContainerGroup();

        free(&jobGroup);
    }

    m_JobContainerGroups.clear();
}

void JobManager::Initialize()
{
    const size_t workerCount = ThreadUtils::GetThreadHardwareCount() - 1;

    m_ThreadPool = new ThreadPool(workerCount);

    ThreadUtils::InitSpinLock(&m_SpinLock);
}

void JobManager::ExecuteWithoutCtx(JobContainer *jobContainer)
{
    if (jobContainer)
    {
        ThreadUtils::LockSpinLock(&m_SpinLock);

        jobContainer->prepareActiveJobs(1);

        ThreadUtils::UnlockSpinLock(&m_SpinLock);

        ParallelProcessor::Range range;
        range.start = 0;
        range.end = 1;

        // 단 한개의 subJob 만을 생성하여 Thread Pool Job 을 실행한다.
        if (JobContainer::JobManagerActiveJob *job =
                jobContainer->getJobWithRange(0, range))
        {
            m_ThreadPool->AddPoolTask(job, nullptr);
        }
    }
}


void JobManager::Execute(JobContext &ctx, JobContainer *jobContainer)
{
    if (jobContainer)
    {
        ThreadUtils::LockSpinLock(&m_SpinLock);

        jobContainer->prepareActiveJobs(1);

        //Allocator가 Thread Safe하지 않아 lock필요
        ThreadUtils::UnlockSpinLock(&m_SpinLock);

        ParallelProcessor::Range scope;
        scope.start = 0;
        scope.end = 1;

        if (JobContainer::JobManagerActiveJob *subJob =
                jobContainer->getJobWithRange(0, scope))
        {
            m_ThreadPool->AddPoolTask(subJob, &ctx);
            ThreadUtils::SetAtomic(&ctx.m_TotalTaskCount, 1);
        }
    }
}

void JobManager::ExecuteParallel(JobContext &ctx,
                                 JobContainer *jobContainer,
                                 size_t totalJobCount,
                                 size_t divideCount)
{
    if (0 == totalJobCount || 0 == divideCount)
    {
        // 실행할 Job 이 없다면 그냥 리턴하게 한다.
        jobContainer->setToRestState();
        return;
    }

    //Create & Add ParallelSubJob
    if (jobContainer)
    {
        // ex) jobCount = 10, divideCount = 3
        // 10 / 3 = 3, 10 % 3 = 1
        // subJobCnt = 3 + 1 = 4
        // 즉, 10개의 job 을 수행해야 하는데, 3개씩 job 을 나누고 싶다면
        // 3개의 job 을 가져가는 Thread 3개, 1개의 Job 을 가져가는 Thread 1개. 이렇게 나뉘는 것이다.
        const size_t remain = totalJobCount % divideCount;

        // const size_t subJobCnt = remain > 0 ? totalJobCount / divideCount + 1
        const size_t threadCount = remain > 0 ? totalJobCount / divideCount + 1
                                              : totalJobCount / divideCount;

        //if SubJobs Count less than len than Create SubJobs
        ThreadUtils::LockSpinLock(&m_SpinLock);

        // threadCount 개수만큼의 Job 을 준비한다.
        // 예를 들어, thread 가 3개라면, job 도 3개를 준비한다.
        // 이를 통해, 각 thread 가 하나의 Job을 수행하는 개념이다.
        // 이때, 각 Thread 가 Execute 하는 함수는, JobContainer 에 정의된 함수이되
        // range 가 다르게 수행한다. 즉, 같은 함수를 실행하지만, 처리하는 데이터 범위가
        // 다르다는 것이다.
        jobContainer->prepareActiveJobs(threadCount); //TODO 개선필요

        ThreadUtils::UnlockSpinLock(&m_SpinLock);

        //Add ParallelSubJobs
        for (size_t i = 0; i < threadCount; ++i)
        {
            uint64_t addedRange = 0;

            // 마지막 SubJob 이라면
            if (i == threadCount - 1)
            {
                // remain 이 0이라는 것은, 다른 Thread 들과 동일한 개수의 일을 시킴
                // remain 이 0보다 크다는 것은, 다른 Thread 들과 다른 개수의 일을 시킴
                addedRange = remain > 0 ? remain : divideCount;
            }
            else
            {
                addedRange = divideCount;
            }

            ParallelProcessor::Range range;
            range.start = divideCount * i;
            range.end = range.start + addedRange;

            JobContainer::JobManagerActiveJob *todoJob =
                jobContainer->getJobWithRange(i, range);

            if (todoJob)
            {
                // 이렇게 얻어온 Job 을 ThreadPool 에게 넘겨서 Thread 에게 일을 시킨다.
                m_ThreadPool->AddPoolTask(todoJob, &ctx);

                // ThreadPool 내부적으로도 m_TotalTaskCount 가 존재하지만, 외부의 ctx 변수를 이용하여
                // 일의 끝나침 등을 파악하고 싶은 것이다.
                // 따라서 ctx.m_TotalTaskCount 값도 증가사켜준다.
                ThreadUtils::AddAtomic(&ctx.m_TotalTaskCount, 1);
            }
        }
    }
}
void JobManager::WaitNoCtx(int32 taskCount)
{
    // taskCount 개수만큼의 일이 끝날 때까지 기다리기
    m_ThreadPool->Wait(taskCount);
}

void JobManager::WaitAll(JobContext &ctx)
{
    int totalTaskCount = ThreadUtils::GetAtomic(&ctx.m_TotalTaskCount);

    while (ThreadUtils::GetAtomic(&ctx.m_FinishTaskCount) < totalTaskCount)
    {
        totalTaskCount = ThreadUtils::GetAtomic(&ctx.m_TotalTaskCount);
    }

    ThreadUtils::AddAtomic(&ctx.m_FinishTaskCount, -totalTaskCount);
    ThreadUtils::AddAtomic(&ctx.m_TotalTaskCount, -totalTaskCount);
}

void JobManager::WaitAllNoCtx()
{
    // 해당 ThreadPool 내의 모든 task 가 끝날 때까지 기다린다.
    m_ThreadPool->WaitAllThreads();
}


void JobManager::ExecuteParallelNoCtx(JobContainer *job,
                                      size_t jobCount,
                                      size_t divideCount)
{
    if (0 == jobCount || 0 == divideCount)
    {
        job->setToRestState();
        return;
    }

    //Create & Add ParallelSubJob
    if (job)
    {
        size_t remain = jobCount % divideCount;
        size_t threadCount =
            remain > 0 ? jobCount / divideCount + 1 : jobCount / divideCount;

        //if SubJobs Count less than len than Create SubJobs
        ThreadUtils::LockSpinLock(&m_SpinLock);

        job->prepareActiveJobs(threadCount); //TODO 개선필요

        ThreadUtils::UnlockSpinLock(&m_SpinLock);

        //Add Active Jobs
        for (size_t i = 0; i < threadCount; ++i)
        {
            uint64_t addedRange = 0;

            if (i == threadCount - 1)
                addedRange = remain > 0 ? remain : divideCount;
            else
                addedRange = divideCount;

            ParallelProcessor::Range range;
            range.start = divideCount * i;
            range.end = range.start + addedRange;
            JobContainer::JobManagerActiveJob *todoJob =
                job->getJobWithRange(i, range);

            if (todoJob)
            {
                m_ThreadPool->AddPoolTask(todoJob, nullptr);
            }
        }
    }
}

void JobManager::Wait(JobContext &ctx, int32 taskCount)
{
    while (ThreadUtils::GetAtomic(&ctx.m_FinishTaskCount) < taskCount)
        ;

    ThreadUtils::AddAtomic(&ctx.m_FinishTaskCount, -taskCount);
    ThreadUtils::AddAtomic(&ctx.m_TotalTaskCount, -taskCount);
}


template <
    class T,
    typename std::enable_if<std::is_base_of<JobContainer, T>::value>::type *>
T *JobManager::MakeThreadJob()
{
    ThreadUtils::InitSpinLock(&m_SpinLock);

    static const uint32 jobCountCapacity = 5;
    static TypeId jobPoolType = Reflection::GetTypeID<JobContainerGroup>();
    static TypeId jobType = Reflection::GetTypeID<T>();

    if (0 == m_JobGroups.count(jobType))
    {
        //없으면 만듬
        JobContainerGroup *jobGroupMemory = reinterpret_cast<JobGroup *>(
            EngineAllocator::Allocate(DataType::RAWDATA,
                                      Reflection::GetTypeSize(jobPoolType),
                                      "JobGroup"));

        //Call Constructor
        JobContainerGroup *jobPool = new (jobGroupMemory) JobGroup(jobType);

        m_JobGroups.insert(std::make_pair(jobType, jobPool));
    }

    //해당 pool에 job 가져옴 return
    JobContainerGroup *jobGroup = m_JobGroups[jobType];

    //TODO 개선필요1
    T *foundJob = jobGroup->GetRestJob<T>(); //pool의 임계영역

    ThreadUtils::pr_spin_unlock(&m_SpinLock);

    return foundJob;
}

} // namespace Hazel