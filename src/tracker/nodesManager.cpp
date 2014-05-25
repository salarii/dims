// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "nodesManager.h"
#include "nodeMedium.h"
#include "trackerMediumsKinds.h"

namespace tracker
{

CNodesManager * CNodesManager::ms_instance = NULL;

CNodesManager*
CNodesManager::getInstance( )
{
	if ( !ms_instance )
	{
		ms_instance = new CNodesManager();
	};
	return ms_instance;
}


CNodesManager::CNodesManager()
{
}

CNodeMedium *
CNodesManager::addNode( CSelfNode * _node )
{
	boost::lock_guard<boost::mutex> lock( m_nodesLock );

	CNodeMedium * medium = new CNodeMedium( _node );
	m_ptrToNodes.insert( std::make_pair( convertToInt( _node ), medium ) );

	return medium;
}

void
CNodesManager::handleMessages()
{

}

bool
CNodesManager::processMessagesFormNode( CSelfNode * _node, std::vector< CMessage > const & _messages )
{
	return true;
}

void
CNodesManager::connectNodes()
{
}

void
CNodesManager::propagateMessage()
{

}

void
CNodesManager::analyseMessage()
{

}

CNodeMedium*
CNodesManager::getMediumForNode( CSelfNode * _node ) const
{
	std::map< unsigned int, CNodeMedium* >::const_iterator iterator = m_ptrToNodes.find( convertToInt( _node ) );
	if ( iterator != m_ptrToNodes.end() )
	{
		return iterator->second;
	}

	return 0;
}

std::list< common::CMedium< TrackerResponses > *>
CNodesManager::provideConnection( int _actionKind, unsigned _requestedConnectionNumber )
{
	std::list< common::CMedium< TrackerResponses > *> mediums;

	std::map< unsigned int, CNodeMedium* >::iterator iterator = m_ptrToNodes.find( ( unsigned int )_actionKind );

	if( iterator != m_ptrToNodes.end() )
	{
		mediums.push_back( iterator->second );
	}
	else if ( CTrackerMediumsKinds::Nodes == _actionKind )
	{
		return m_nodeMediums;
	}

	return mediums;
}

}
