#ifndef TASK_QUEUE_CONTAINER_H
#define TASK_QUEUE_CONTAINER_H

namespace Self
{

struct CQueueType
{
	enum Enum
	{
		  MONITOR_TASK = 0
		, TRACKER_TASK
		, NODE_TASK
	};
};


template < CQueueType::Enum _A, CQueueType::Enum _B, CQueueType::Enum _C >
class CPriority
{
public:
	struct First
	{
		static const unsigned id = _A;
	};

	struct Second
	{
		static const unsigned id = _B;
	};

	struct Third
	{
		static const unsigned id = _C;
	};
};

typedef CPriority< CQueueType::MONITOR_TASK, CQueueType::NODE_TASK, CQueueType::TRACKER_TASK > Priority;

typedef std::vector< std::list > TaskQueues;

//thread
class CTaskQueueContainer : public CTaskQueue
{
	//lock
	CTaskQueueContainer();
	//
	void proccessTask();

	getWatermark() const;

private:
	
	putRequestIntoCommunicationChannel( boost::variant< some Stuff > )
private:
	boost::signal<> m_immediateStallAll;

	m_communicationIntoChannel;

	TaskQueues m_taskQueues;
};
}

#endif // TASK_QUEUE_CONTAINER_H
