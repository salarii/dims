#include "manageNetwork.h"

namespace self
{

CManageNetwork::CManageNetwork()
{
}

void 
CManageNetwork::startClientServer()
{
	Poco::Net::ServerSocket serverSocket( m_networkParams->GetDefaultPort() );

	//Configure some server params.
	Poco::Net::TCPServerParams* pParams = new Poco::Net::TCPServerParams();
	pParams->setMaxThreads(4);
	pParams->setMaxQueued(10);
	pParams->setThreadIdleTime(50);

	//Create your server
	m_tcpServer = new Poco::Net::TCPServer(new Poco::Net::TCPServerConnectionFactoryImpl<newConnection>(), serverSocket, pParams);
	m_tcpServer->start();
}

}