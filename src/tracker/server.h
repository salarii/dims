// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include "Poco/Net/TCPServer.h"
#include "Poco/Net/TCPServerParams.h"
#include "Poco/Net/TCPServerConnectionFactory.h"
#include "Poco/Net/TCPServerConnection.h"
#include "Poco/Net/Socket.h"

namespace self
{

class CTcpServerConnection : public Poco::Net::TCPServerConnection
{
public:
	struct RespondBuffor
	{
		char *m_buffor;
		unsigned int m_size;
	};
public:
	CTcpServerConnection(Poco::Net::StreamSocket const & _serverConnection ) :
	Poco::Net::TCPServerConnection( _serverConnection )
	{
	}

	bool handleIncommingBuffor( unsigned char* _buffor, unsigned int _size );

	void run();
private:
	template < class T >
	void
	handleMessage( std::vector< T > const & _messages, RespondBuffor & _respondBuffor );
private:
	CNetworkParams * m_networkParams;
};
//create identification token 
// send back identification token 

#endif