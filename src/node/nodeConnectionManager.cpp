#include "nodeConnectionManager.h"

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

CNodeConnectionManager* CNodeConnectionManager::getInstance()
{
	return ms_instance;
}



void
CNodeConnectionManager::connectToNetwork()
{
// get seed  - not used yet
// get monitors - not used yet 
// get  trackers - not  used  yet 
// get tracker
	m_actionHandler
}

void
CNodeConnectionManager::executeAction()
{


}

}
