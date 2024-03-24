#pragma once

#include "Hazel/Core/Thread/ThreadTask.h"
#include "Hazel/Core/Thread/ParallelProcessor.h"

// https://wickedengine.net/2018/11/24/simple-job-system-using-standard-c/

namespace Hazel
{
// Job Class �� '����Լ�' �� ���� Task class template
// �ش� ����Լ��� ����Ÿ���� void, ���ڴ� void* �̴�.
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

// ParallelJob �� ActiveJob �� ���̰� ���� ?
// Naming �� �ٸ��� �ؾ� �� �� ����.
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

    // �ش� ParallelJob ����, ��ü ������ ���� ����.
    size_t m_Length;

    void dispatch(void *args);
};

/**
��ǻ� ���� Job �� �ƴ϶� , Job �� �����ϴ� ���� SubJob ���� ���
������ Container ����
���� SubJob ���� ������ Method ���� �����صΰ�, �� Job ����
������ �ٸ��� �Ͽ� �ش� �Լ��� �����ϱ� ���� Class
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
	* JobSystemJob::Execute���� ����Ǵ� �޼ҵ� �̴�.
	* ��, ���� ���� Job ���� ���� �Լ��� �����ϵ�, range �� �ٸ��� �����Ѵ�.
	*/
    virtual void ExecuteMainJob(const ParallelProcessor::Range &range) = 0;

public:
    JobContainer();
    virtual ~JobContainer();

    /**
JobManager �� ThreadPool �ȿ� �ִ� Thread ����
�����ϴ� ���� Job ��
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
			* @brief LvJobSystem���� ����Ǵ� �޼ҵ�
				- member���� _range�� args�� �̿��� LvEngineJob::Execute����
				- ��� subJob�� �Ϸ�Ǹ� mainJob�� Rest���·� ��ȯ��
			* @param args LvEngineJob::Execute�� �����ϱ� ���� ������������ �ش� Ŭ�������� ������� ����
		*/
        void Execute(void *args) override;

        /**
			* @brief Job Group�� ����, �� index�� �����ϴ� �Լ�
			* @param range Job Group�� ���� �� index�� ���� ����
				- LvEngineJobSystem���� ��������
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
		* @brief ActiveJob�� �����ϴ� �޼ҵ�
	*/
    void addJobs();
    /**
		* @brief LvEngineJobSystem���� ����� Group������ ActiveJob�� ���� ��� ActiveJob�� �� ����� �޼ҵ�
		* @param len LvEngineJobSystem���� ����� Group��
	*/
    void prepareJobs(size_t len);
    /**
		* @brief ActiveJob�� _range�� �ʱ�ȭ �Ͽ� �������� �޼ҵ�
		* index :  index ������ ActiveJob�� index
		* @param range ���� ActiveJob�� ������ Group�� ����, �� index
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
        m_WorkingFlag; // �ش� JobContainer �� Rest ��������, Running ��������
    Atomic
        m_WorkCount; // �� ������ work �� ���� (1�� work �� 1���� subJob �� ���� ?)
};
} // namespace Hazel