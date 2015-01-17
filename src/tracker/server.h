// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SERVER_H
#define SERVER_H

#include <iostream>

#include "Poco/Net/TCPServerParams.h"
#include "Poco/Net/TCPServerConnection.h"
#include "Poco/Net/Socket.h"

#include "common/communicationBuffer.h"

class CBufferAsStream;

namespace tracker
{
class CClientRequestsManager;

void runServer();

class CTcpServerConnection : public Poco::Net::TCPServerConnection
{
public:
	CTcpServerConnection(Poco::Net::StreamSocket const & _serverConnection );

	bool handleIncommingBuffor();

	void run();
private:
	void writeSignature( CBufferAsStream & _stream );

	bool checkSignature( CBufferAsStream const & _stream );
/*
	template < class T >
	void
	handleMessage( std::vector< T > const & _messages, RespondBuffor & _respondBuffor );
*/
private:
	common::CCommunicationBuffer m_pullBuffer;

	common::CCommunicationBuffer m_pushBuffer;

	CClientRequestsManager * m_clientRequestManager;
};



//create identification token 
// send back identification token 
}
#endif
