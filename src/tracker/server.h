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
	CTcpServerConnection(Poco::Net::StreamSocket const & _serverConnection ) :
	Poco::Net::TCPServerConnection( _serverConnection )
	{
	}

	void run();
};

#endif