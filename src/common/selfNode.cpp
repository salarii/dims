// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/selfNode.h"
#include "common/nodesManager.h"

namespace common
{

void
CSelfNode::setMessagesToSend( std::vector< CMessage > const & _messages )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );
	m_messagesToSend.insert( m_messagesToSend.end(), _messages.begin(), _messages.end() );
}

void
CSelfNode::setMessageToSend( CMessage const & _message )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );
	m_messagesToSend.push_back( _message );
}

void
CSelfNode::sendMessages()
{
	boost::lock_guard<boost::mutex> lock( m_mutex );

	if ( !m_messagesToSend.empty() )
		PushMessage("dims", m_messagesToSend);

	m_messagesToSend.clear();
}

CSelfNode::~CSelfNode()
{
	LogPrintf("delete node %s \n", addrName );
	clearManager();
}

}
