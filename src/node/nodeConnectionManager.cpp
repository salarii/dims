#include "nodeConnectionManager.h"
#include "common/actionHandler.h"
#include "connectAction.h"
#include "settingsConnectionProvider.h"
#include "trackerLocalRanking.h"
#include "util.h"
#include "common/connectionProvider.h"

namespace node
{

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
	m_actionHandler = common::CActionHandler< NodeResponses >::getInstance();

	m_actionHandler->addConnectionProvider( CSettingsConnectionProvider::getInstance() );
	
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
CNodeConnectionManager::executeAction()
{


}

}
