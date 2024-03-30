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
    // ThreadPool ���� Thread ���� ��� ����� ������ ��ٸ���.
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

        // �� �Ѱ��� subJob ���� �����Ͽ� Thread Pool Job �� �����Ѵ�.
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

        //Allocator�� Thread Safe���� �ʾ� lock�ʿ�
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
        // ������ Job �� ���ٸ� �׳� �����ϰ� �Ѵ�.
        jobContainer->setToRestState();
        return;
    }

    //Create & Add ParallelSubJob
    if (jobContainer)
    {
        // ex) jobCount = 10, divideCount = 3
        // 10 / 3 = 3, 10 % 3 = 1
        // subJobCnt = 3 + 1 = 4
        // ��, 10���� job �� �����ؾ� �ϴµ�, 3���� job �� ������ �ʹٸ�
        // 3���� job �� �������� Thread 3��, 1���� Job �� �������� Thread 1��. �̷��� ������ ���̴�.
        const size_t remain = totalJobCount % divideCount;

        // const size_t subJobCnt = remain > 0 ? totalJobCount / divideCount + 1
        const size_t threadCount = remain > 0 ? totalJobCount / divideCount + 1
                                              : totalJobCount / divideCount;

        //if SubJobs Count less than len than Create SubJobs
        ThreadUtils::LockSpinLock(&m_SpinLock);

        // threadCount ������ŭ�� Job �� �غ��Ѵ�.
        // ���� ���, thread �� 3�����, job �� 3���� �غ��Ѵ�.
        // �̸� ����, �� thread �� �ϳ��� Job�� �����ϴ� �����̴�.
        // �̶�, �� Thread �� Execute �ϴ� �Լ���, JobContainer �� ���ǵ� �Լ��̵�
        // range �� �ٸ��� �����Ѵ�. ��, ���� �Լ��� ����������, ó���ϴ� ������ ������
        // �ٸ��ٴ� ���̴�.
        jobContainer->prepareActiveJobs(threadCount); //TODO �����ʿ�

        ThreadUtils::UnlockSpinLock(&m_SpinLock);

        //Add ParallelSubJobs
        for (size_t i = 0; i < threadCount; ++i)
        {
            uint64_t addedRange = 0;

            // ������ SubJob �̶��
            if (i == threadCount - 1)
            {
                // remain �� 0�̶�� ����, �ٸ� Thread ��� ������ ������ ���� ��Ŵ
                // remain �� 0���� ũ�ٴ� ����, �ٸ� Thread ��� �ٸ� ������ ���� ��Ŵ
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
                // �̷��� ���� Job �� ThreadPool ���� �Ѱܼ� Thread ���� ���� ��Ų��.
                m_ThreadPool->AddPoolTask(todoJob, &ctx);

                // ThreadPool ���������ε� m_TotalTaskCount �� ����������, �ܺ��� ctx ������ �̿��Ͽ�
                // ���� ����ħ ���� �ľ��ϰ� ���� ���̴�.
                // ���� ctx.m_TotalTaskCount ���� ���������ش�.
                ThreadUtils::AddAtomic(&ctx.m_TotalTaskCount, 1);
            }
        }
    }
}
void JobManager::WaitNoCtx(int32 taskCount)
{
    // taskCount ������ŭ�� ���� ���� ������ ��ٸ���
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
    // �ش� ThreadPool ���� ��� task �� ���� ������ ��ٸ���.
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

        job->prepareActiveJobs(threadCount); //TODO �����ʿ�

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
        //������ ����
        JobContainerGroup *jobGroupMemory = reinterpret_cast<JobGroup *>(
            EngineAllocator::Allocate(DataType::RAWDATA,
                                      Reflection::GetTypeSize(jobPoolType),
                                      "JobGroup"));

        //Call Constructor
        JobContainerGroup *jobPool = new (jobGroupMemory) JobGroup(jobType);

        m_JobGroups.insert(std::make_pair(jobType, jobPool));
    }

    //�ش� pool�� job ������ return
    JobContainerGroup *jobGroup = m_JobGroups[jobType];

    //TODO �����ʿ�1
    T *foundJob = jobGroup->GetRestJob<T>(); //pool�� �Ӱ迵��

    ThreadUtils::pr_spin_unlock(&m_SpinLock);

    return foundJob;
}

} // namespace Hazel