#include "nodeConnectionManager.h"
#include "actionHandler.h"
#include "sendInfoRequestAction.h"

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
	m_actionHandler = CActionHandler::getInstance();
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

	CSendInfoRequestAction * sendInfoRequestAction = new CSendInfoRequestAction( NetworkInfo::Tracker );

	m_actionHandler->executeAction( sendInfoRequestAction );

}

void
CNodeConnectionManager::executeAction()
{


}

}
