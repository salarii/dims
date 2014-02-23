#ifndef COMMUNICATION_CHANNEL_RESOLVER_H
#define COMMUNICATION_CHANNEL_RESOLVER_H

namespace Self
{

class CTask;

template <list of all modules  >
class RedirectPoint : public boost::static_visitor<bool>
{
public:
	CTaskQueue & operator()(module  & module_1 )
	{
		// getWorkQueueOf givenModule
		// add task  to  this  work queue
		return true;
	}

	CTaskQueue & operator()( module & module_2 )
	{
		// getWorkQueueOf givenModule
		// add task  to  this  work queue
		return true;
	}

	CTaskQueue & operator()( module & module_n )
	{
		// getWorkQueueOf givenModule
		// add task  to  this  work queue
		return true;
	}
};

// those request are send from one module of an app to another
// those have nothing in common with networking 

template < class Target, class Work > 
class TaskRequest
{
	boost::variant<  > m_destiny;
	CTask & m_task;
};

class CCommunicationChannelResolver
{
	
void redirectTask( TaskRequest & _internalAppTaskRequest )
{
	boost::apply_visitor( RedirectPoint(), TaskRequest.m_destiny ).addTask( TaskRequest );
}
private:

};

}

#endif
