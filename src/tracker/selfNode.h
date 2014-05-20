// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SELF_NODE_H
#define SELF_NODE_H

#include "net.h"
#include "key.h"
#include "communicationProtocol.h"

namespace tracker
{

class CSelfNode : public CNode
{
public:
	void setMessageToSend( CMessage const & _message );

	void sendMessages();
private:
	mutable boost::mutex mutex;
	std::vector< CMessage > m_messagesToSend;
	std::vector< CMessage > m_receivedMessages;
};

}

#endif
