#include "Hazel/Core/Thread/Jobs.h"
#include "Hazel/Core/Thread/ThreadUtil.h"
#include "Hazel/Core/Thread/JobSystem.h"

namespace Hazel
{
Job::Job()
    // &Job::Execute 를 넘겨주면 MemberFuncPointer m_MemberFunc 에 호출할 함수를 설정해주는 것.
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
    // 현재로서는 _index 라는 변수는 사용되지 않는 것 같다.
    // Parallel::For(m_Index, m_Length, [=](Parallel::Scope rng)
    // {
    // 	//...? 무한 루프 호출하는 거 아닌가..?
    // 	// 인자로 들어오는 range 는 왜 안쓰고, rng 라는 인자만 사용하게 되는거지 ?
    // 	// 함수의 API 를 변경해야 할 것 같다.
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

    // 총 수행할 work 개수만큼의 subJob 들을 추가적으로 만들어낸다.
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

    // 보니 애초에 m_Jobs 에 일정 Job 개수를 세팅해두고
    // getSubJob 함수를 호출할 때마다 range 라는 변수를 새로 세팅해준다.
    // 이 의미는, getSubJob 함수를 호출할 때마다 해당 Job 이 실행할 함수 및 범위를
    // 새로 매번 설정해준다는 것이다.
    m_JobManagerJobs[index]->SetRange(range);

    return m_JobManagerJobs[index];
}

void JobContainer::JobManagerActiveJob::Execute(void *args)
{
    // 모든 subJob 들이 같은 함수를 실행한다. 하지만 내부적으로 m_Range 변수에 따라
    // 다른 일을 수행하게 될 것이다.
    m_JobContainer->ExecuteMainJob(m_Range);

    if (nullptr != args)
    {
        JobContext *ctx = (JobContext *)args;

        // m_FinishTaskCount 를 1 증가시킨다.
        // ctx 는, 해당 ActiveJob 이 속한 JobContainer 뿐만 아니라
        // 전체 Job 들이 담긴 JobSystem 의 FinishedTaskCnt 를 증가시키는 것이다.
        ThreadUtils::IncreaseAtomic(&ctx->m_FinishTaskCount);
    }

    // 각 ActiveJob 들이 일을 끝낼 때마다 m_WorkCount 를 1 씩 감소시킨다.
    // 0 이 된다는 것은, 더이상 할일이 남아있지 않다는 것이므로
    // 다시 mainJob 을 rest 상태로 만든다.
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
    // m_WorkCount 를 1 감소시킨다.
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
    // REST 상태로 다시 바꿔준다. 그리고 이전 상태값을 리턴해준다.
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