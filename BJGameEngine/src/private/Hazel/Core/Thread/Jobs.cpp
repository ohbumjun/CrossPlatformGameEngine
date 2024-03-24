#include "Hazel/Core/Thread/Jobs.h"
#include "Hazel/Core/Thread/ThreadUtil.h"
#include "Hazel/Core/Thread/JobSystem.h"

namespace Hazel
{
Job::Job()
    // &Job::Execute �� �Ѱ��ָ� MemberFuncPointer m_MemberFunc �� ȣ���� �Լ��� �������ִ� ��.
    : BaseClass(this, &Job::Execute)
{
}

ParallelJob::ParallelJob(ParallelJob &&o)
    : BaseClass(this, o), m_Index(o.m_Index), m_Length(o.m_Length)
{
}

ParallelJob::ParallelJob(size_t index, size_t length)
    : BaseClass(this, &ParallelJob::dispatch), m_Index(index),
      m_Length(m_Length)
{
}

void ParallelJob::dispatch(void *args)
{
    // ����μ��� _index ��� ������ ������ �ʴ� �� ����.
    // Parallel::For(m_Index, m_Length, [=](Parallel::Scope rng)
    // {
    // 	//...? ���� ���� ȣ���ϴ� �� �ƴѰ�..?
    // 	// ���ڷ� ������ range �� �� �Ⱦ���, rng ��� ���ڸ� ����ϰ� �Ǵ°��� ?
    // 	// �Լ��� API �� �����ؾ� �� �� ����.
    // 	// Execute(rng, args);
    // });
}

JobContainer::JobContainer()
{
    ThreadUtils::SetAtomic(&m_WorkingFlag, (int)JobContainer::State::Rest);
}

JobContainer::~JobContainer()
{
    for (JobManagerActiveJob *task : m_JobManagerJobs)
    {
        task->~JobManagerActiveJob();
        free(task);
        // EngineAllocator::Free(task);
    }
}

void JobContainer::addJobs()
{
    // JobSystemJob *r =
    //     (JobSystemJob *)EngineAllocator::Allocate(DataType::RAWDATA,
    //                                               sizeof(ActiveJob),
    //                                               JobSystemJob::TypeName());
    //
    // JobSystemJob *subJob = new (r) JobSystemJob(this);

    JobManagerActiveJob *subJob = new JobManagerActiveJob(this);

    m_JobManagerJobs.push_back(subJob);
}

void JobContainer::prepareJobs(size_t len)
{
    setWorkCount((int)len);

    if (len < m_JobManagerJobs.size())
        return;

    size_t prevSubTaskNum = m_JobManagerJobs.size();

    // �� ������ work ������ŭ�� subJob ���� �߰������� ������.
    for (size_t i = 0; i < len - prevSubTaskNum; ++i)
    {
        addJobs();
    }
}

JobContainer::JobManagerActiveJob *JobContainer::getJobWithRange(
    size_t index,
    ParallelProcessor::Range &range)
{
    if (m_JobManagerJobs.size() <= index)
    {
        // assert(false, "warnning : LvEngineJob::getSubJob - _subJobs length < index");
        assert(false);
        return nullptr;
    }

    // ���� ���ʿ� m_Jobs �� ���� Job ������ �����صΰ�
    // getSubJob �Լ��� ȣ���� ������ range ��� ������ ���� �������ش�.
    // �� �ǹ̴�, getSubJob �Լ��� ȣ���� ������ �ش� Job �� ������ �Լ� �� ������
    // ���� �Ź� �������شٴ� ���̴�.
    m_JobManagerJobs[index]->SetRange(range);

    return m_JobManagerJobs[index];
}

void JobContainer::JobManagerActiveJob::Execute(void *args)
{
    // ��� subJob ���� ���� �Լ��� �����Ѵ�. ������ ���������� m_Range ������ ����
    // �ٸ� ���� �����ϰ� �� ���̴�.
    m_JobContainer->ExecuteMainJob(m_Range);

    if (nullptr != args)
    {
        JobContext *ctx = (JobContext *)args;

        // m_FinishTaskCount �� 1 ������Ų��.
        // ctx ��, �ش� ActiveJob �� ���� JobContainer �Ӹ� �ƴ϶�
        // ��ü Job ���� ��� JobSystem �� FinishedTaskCnt �� ������Ű�� ���̴�.
        ThreadUtils::IncreaseAtomic(&ctx->m_FinishTaskCount);
    }

    // �� ActiveJob ���� ���� ���� ������ m_WorkCount �� 1 �� ���ҽ�Ų��.
    // 0 �� �ȴٴ� ����, ���̻� ������ �������� �ʴٴ� ���̹Ƿ�
    // �ٽ� mainJob �� rest ���·� �����.
    if (m_JobContainer->isAllSubJobDone())
    {
        m_JobContainer->setReadyRestState();
    }
}

void JobContainer::setWorkCount(int value)
{
    if (value < 0)
    {
        // assert(false, "error : setWorkCount value less than 0");
        assert(false);
    }

    ThreadUtils::SetAtomic(&m_WorkCount, value);
}

bool JobContainer::isAllSubJobDone()
{
    // m_WorkCount �� 1 ���ҽ�Ų��.
    int leftWorkCount = ThreadUtils::DecreaseAtomic(&m_WorkCount);

    if (leftWorkCount < 0)
    {
        // assert(false, "error : leftWorkCount less than 0");
        assert(false);
    }

    if (leftWorkCount == 0)
        return true;

    return false;
}

bool JobContainer::setReadyRestState()
{
    // REST ���·� �ٽ� �ٲ��ش�. �׸��� ���� ���°��� �������ش�.
    JobContainer::State beforeState = (JobContainer::State)
        ThreadUtils::SetAtomic(&m_WorkingFlag, (int)JobContainer::State::Rest);

    if (beforeState == JobContainer::State::Work)
    {
        return true;
    }

    return false;
}

bool JobContainer::setWorkState()
{
    JobContainer::State beforeState = (JobContainer::State)
        ThreadUtils::SetAtomic(&m_WorkingFlag, (int)JobContainer::State::Work);

    if (beforeState == JobContainer::State::Rest)
    {
        return true;
    }

    return false;
}
} // namespace Hazel