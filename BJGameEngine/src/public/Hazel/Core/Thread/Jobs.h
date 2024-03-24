#pragma once

#include "Hazel/Core/Thread/ThreadTask.h"
#include "Hazel/Core/Thread/ParallelProcessor.h"

// https://wickedengine.net/2018/11/24/simple-job-system-using-standard-c/

namespace Hazel
{
// Job Class 의 '멤버함수' 에 대한 Task class template
// 해당 멤버함수는 리턴타입이 void, 인자는 void* 이다.
class Job : public ThreadTask<void (Job::*)(void *)>
{
    typedef ThreadTask<void (Job::*)(void *)> BaseClass;

public:
    Job();

    virtual ~Job()
    {
    }

    virtual void Execute(void *args) = 0;
};

// ParallelJob 과 ActiveJob 의 차이가 뭐지 ?
// Naming 을 다르게 해야 할 것 같다.
class ParallelJob : public ThreadTask<void (ParallelJob::*)(void *)>
{
    typedef ThreadTask<void (ParallelJob::*)(void *)> BaseClass;

public:
    ParallelJob() = delete;

    ParallelJob(ParallelJob &&o);

    explicit ParallelJob(size_t index, size_t length);

    virtual void Execute(ParallelProcessor::Range range, void *args) = 0;

private:
    size_t m_Index;

    // 해당 ParallelJob 말고, 전체 실행할 일의 개수.
    size_t m_Length;

    void dispatch(void *args);
};

/**
사실상 실제 Job 이 아니라 , Job 을 실행하는 여러 SubJob 들을 담는
일종의 Container 역할
여러 SubJob 들이 수행할 Method 들을 정의해두고, 각 Job 들이
범위만 다르게 하여 해당 함수를 실행하기 위한 Class
*/
class JobContainer
{
    friend class ManagedJobSystem;
    friend class JobGroup;
    friend class JobManager;

public:
    enum class State : int
    {
        Work = 0,
        Rest = 1
    };

    /**
	* JobSystemJob::Execute에서 수행되는 메소드 이다.
	* 즉, 여러 개의 Job 들이 같은 함수를 실행하되, range 만 다르게 수행한다.
	*/
    virtual void ExecuteMainJob(const ParallelProcessor::Range &range) = 0;

public:
    JobContainer();
    virtual ~JobContainer();

    /**
JobManager 의 ThreadPool 안에 있는 Thread 들이
실행하는 실제 Job 들
*/
private:
    class JobManagerActiveJob : public Job
    {
    public:
        JobManagerActiveJob(JobContainer *mainJob)
            : Job(), m_JobContainer(mainJob)
        {
        }

        ~JobManagerActiveJob() override = default;

        /**
			* @brief LvJobSystem에서 실행되는 메소드
				- member변수 _range와 args를 이용해 LvEngineJob::Execute수행
				- 모든 subJob이 완료되면 mainJob을 Rest상태로 전환함
			* @param args LvEngineJob::Execute를 수행하기 위한 데이터이지만 해당 클래스에선 사용하지 않음
		*/
        void Execute(void *args) override;

        /**
			* @brief Job Group의 시작, 끝 index를 설정하는 함수
			* @param range Job Group의 시작 끝 index를 가진 변수
				- LvEngineJobSystem에서 설정해줌
		*/
        void SetRange(const ParallelProcessor::Range &range)
        {
            m_Range.start = range.start;
            m_Range.end = range.end;
        }

        static std::string TypeName()
        {
            return "ActiveJob";
        }

    private:
        JobContainer *m_JobContainer;
        ParallelProcessor::Range m_Range;
    };

    /**
		* @brief ActiveJob을 제작하는 메소드
	*/
    void addJobs();
    /**
		* @brief LvEngineJobSystem에서 계산한 Group수보다 ActiveJob이 적을 경우 ActiveJob를 더 만드는 메소드
		* @param len LvEngineJobSystem에서 계산한 Group수
	*/
    void prepareJobs(size_t len);
    /**
		* @brief ActiveJob의 _range를 초기화 하여 가져오는 메소드
		* index :  index 가져올 ActiveJob의 index
		* @param range 현재 ActiveJob가 수행할 Group의 시작, 끝 index
	*/
    JobContainer::JobManagerActiveJob *getJobWithRange(
        size_t index,
        ParallelProcessor::Range &range);

    void setWorkCount(int value);
    bool isAllSubJobDone();
    bool setReadyRestState();
    bool setWorkState();

private:
    std::vector<JobManagerActiveJob *> m_JobManagerJobs;

    Atomic
        m_WorkingFlag; // 해당 JobContainer 가 Rest 상태인지, Running 상태인지
    Atomic
        m_WorkCount; // 총 수행할 work 의 개수 (1개 work 를 1개의 subJob 이 수행 ?)
};
} // namespace Hazel