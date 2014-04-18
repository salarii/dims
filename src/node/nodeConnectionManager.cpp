#include "nodeConnectionManager.h"
#include "actionHandler.h"
#include "connectAction.h"
#include "userConnectionProvider.h"
#include "trackerLocalRanking.h"
#include "util.h"


namespace node
{
// for testing purposes low value, so maybe  use  some  macro  defined in  makefile at some  point  to  implement  different  values  for  test and main
unsigned int const CNodeConnectionManager::m_sleepTime = 10000;

CNodeConnectionManager * CNodeConnectionManager::ms_instance = NULL;

CNodeConnectionManager*
CNodeConnectionManager::getInstance( )
{
	if ( !ms_instance )
	{
		ms_instance = new CNodeConnectionManager();
	};
	return ms_instance;
}

CNodeConnectionManager::CNodeConnectionManager()
{
	m_actionHandler = CActionHandler::getInstance();

	m_actionHandler->addConnectionProvider( new CUserConnectionProvider() );
	
	m_actionHandler->addConnectionProvider( CTrackerLocalRanking::getInstance() );
}


CNodeConnectionManager::~CNodeConnectionManager()
{
	if ( ms_instance )
		delete ms_instance;
	ms_instance = 0;
}

void
CNodeConnectionManager::connectToNetwork()
{
// get seed  - not used yet
// get monitors - not used yet 
// get  trackers - not  used  yet 
// get tracker

	CConnectAction * connectAction = new CConnectAction( CConnectAction::State::Manual );

	m_actionHandler->executeAction( connectAction );

}

void
CNodeConnectionManager::periodicActionLoop()
{
	while(1)
	{
		{
			boost::lock_guard<boost::mutex> lock( m_mutex );
			BOOST_FOREACH(CAction* action, m_periodicActions)
			{
				if ( action->getState() != ActionStatus::InProgress )
				{
					if ( action->getState() == ActionStatus::Done )
						action->reset();

					m_actionHandler->executeAction( action );
				}
			}
		}
		MilliSleep(m_sleepTime );
	}
}
	
void
CNodeConnectionManager::addPeriodicAction( CAction* _action )
{
	boost::lock_guard<boost::mutex> lock(m_mutex);
	m_periodicActions.push_back( _action );
}

void
CNodeConnectionManager::executeAction()
{


}

}
