// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MANAGE_NETWORK_H
#define MANAGE_NETWORK_H 

class CManageNetwork
{
public:
	CManageNetwork();

	void startClientServer();

	mainLoop();

	/*
	validate  buffer 
	
	*/
private:
	void connectToNetwork();
	void negotiateWithMonitor();
// pointer to network task queue
	CNetworkParams * m_networkParams;

	Poco::Net::TCPServer  * m_tcpServer;
};


#endif // MANAGE_NETWORK_H