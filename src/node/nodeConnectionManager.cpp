#include "nodeConnectionManager.h"
#include "actionHandler.h"
#include "connectAction.h"
#include "userConnectionProvider.h"
#include "trackerLocalRanking.h"
#include "util.h"
#include "common/connectionProvider.h"

namespace node
{
// any periodic  acton should be ecapsulated in  this  way
/*
class CDefferAction
{
public:
CDefferAction( CAction*, unsigned int _deffer )
{

}

bool isReady();

void execute();
private:
unsigned int m_deffer;
unsigned int m_cnt;
};

bool
CDefferAction::isReady()
{
if(!m_cnt--)
{
m_cnt = m_deffer
return true;
}
return false;
}

void
CDefferAction::execute()
{


}

*/
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
	static  int i = 2;
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

					if ( i == 0 )
						return;
					i--;
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
