// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "selfNode.h"

namespace tracker
{

void
CSelfNode::setMessageToSend( CMessage const & _message )
{
	boost::lock_guard<boost::mutex> lock( mutex );
	m_messagesToSend.push_back( _message );
}

void
CSelfNode::sendMessages()
{
	boost::lock_guard<boost::mutex> lock( mutex );

	PushMessage("", m_messagesToSend);

	m_messagesToSend.clear();
}

}
