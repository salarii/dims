// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SELF_NODE_H
#define SELF_NODE_H

#include "net.h"
#include "key.h"
#include "communicationProtocol.h"

namespace common
{

class CSelfNode : public CNode
{
public:
	CSelfNode(SOCKET hSocketIn, CAddress addrIn, std::string addrNameIn = "", bool fInboundIn=false) : CNode(hSocketIn, addrIn, addrNameIn, fInboundIn){};

	void setMessageToSend( CMessage const & _message );

	void setMessagesToSend( std::vector< CMessage > const & _messages );

	void sendMessages();
private:
	mutable boost::mutex m_mutex;
	std::vector< CMessage > m_messagesToSend;
	std::vector< CMessage > m_receivedMessages;
};

}

#endif
