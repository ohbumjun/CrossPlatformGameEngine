#pragma once

#include "Hazel/Core/Thread/ThreadTask.h"
#include "Hazel/Core/Thread/ParallelProcessor.h"

// https://wickedengine.net/2018/11/24/simple-job-system-using-standard-c/

namespace Hazel
{
// Job Class �� '����Լ�' �� ���� Task class template
// �ش� ����Լ��� ����Ÿ���� void, ���ڴ� void* �̴�.
class BaseJob : public ThreadTask<void (BaseJob::*)(void *)>
{
    typedef ThreadTask<void (BaseJob::*)(void *)> BaseClass;

public:
    BaseJob();

    virtual ~BaseJob()
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
        RUN = 0,
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
    class JobManagerActiveJob : public BaseJob
    {
    public:
        JobManagerActiveJob(JobContainer *mainJob)
            : BaseJob(), m_JobContainer(mainJob)
        {
        }

        ~JobManagerActiveJob() override = default;

        /**
			* JobSystem���� ����Ǵ� �Լ�
				- member���� _range�� args�� �̿��� �� Job Class �� Execute �Լ��� �����Ѵ�.
                - ���� ��� ���� ������, REST ���·� ��ȯ�Ѵ�.
				- ��� subJob�� �Ϸ�Ǹ� mainJob�� Rest���·� ��ȯ��
		*/
        void Execute(void *args) override;

        /**
			* @brief Job Group�� ����, �� index�� �����ϴ� �Լ�
			* @param range Job Group�� ���� �� index�� ���� ����
				- LvEngineJobSystem���� ��������
		*/
        void SetExecuteRange(const ParallelProcessor::Range &range)
        {
            m_Range.start = range.start;
            m_Range.end = range.end;
        }

        static std::string TypeName()
        {
            return "JobManagerActiveJob";
        }

    private:
        JobContainer *m_JobContainer;
        ParallelProcessor::Range m_Range;
    };

    /**
		* @brief ActiveJob�� �����ϴ� �޼ҵ�
	*/
    void addActiveJobs();
    /**
		* @brief LvEngineJobSystem���� ����� Group������ ActiveJob�� ���� ��� ActiveJob�� �� ����� �޼ҵ�
		* @param len LvEngineJobSystem���� ����� Group��
	*/
    void prepareActiveJobs(size_t len);
    /**
		* ActiveJob�� _range�� �ʱ�ȭ �Ͽ� �������� �Լ�
		* - index ������ ActiveJob�� index
		* - range ���� ActiveJob�� ������ Group�� ����, �� index
	*/
    JobContainer::JobManagerActiveJob *getJobWithRange(
        size_t index,
        ParallelProcessor::Range &range);

    void setWorkCount(int value);
    bool isAllJobDone();
    bool setToRestState();
    bool setToRunState();

private:
    std::vector<JobManagerActiveJob *> m_JobManagerJobs;

    Atomic
        m_WorkingFlag; // �ش� JobContainer �� Rest ��������, Running ��������
    Atomic
        m_WorkCount; // �� ������ work �� ���� (1�� work �� 1���� subJob �� ���� ?)
};
} // namespace Hazel