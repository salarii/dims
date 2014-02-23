#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

namespace Self
{

class CTask;

class CTaskQueue
{
public: 
	virtual addTask( CTask & _task ) = 0;

	lock(){ m_lock.lock(); }
	unlock(){ m_lock.unlock(); };
private:
	mutable boost::mutex m_lock;
};


}

#endif