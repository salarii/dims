// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/nodesManager.h"

namespace common
{

CNodesManager * common::CNodesManager::ms_instance = 0;

CNodesManager ::CNodesManager()
{
}

void
CNodesManager ::addNode( CNodeMedium * _medium )
{
	boost::lock_guard<boost::mutex> lock( m_nodesLock );
// create  and  run ping  action
	m_ptrToNodes.insert( std::make_pair( convertToInt( _medium->getNode() ), _medium ) );
}

CNodeMedium*
CNodesManager ::getMediumForNode( common::CSelfNode * _node ) const
{
	typename std::map< uintptr_t, CNodeMedium* >::const_iterator iterator = m_ptrToNodes.find( convertToInt( _node ) );
	if ( iterator != m_ptrToNodes.end() )
	{
		return iterator->second;
	}

	return 0;
}

std::list< CMedium*>
CNodesManager ::provideConnection( CMediumFilter const & _mediumFilter )
{
	return _mediumFilter.getMediums( this );
}

 CMedium *
CNodesManager ::findNodeMedium( uintptr_t _ptr ) const
{
	typename std::map< uintptr_t, CNodeMedium* >::const_iterator iterator = m_ptrToNodes.find( _ptr );

	return iterator != m_ptrToNodes.end() ? iterator->second : 0;
}

void
CNodesManager ::eraseMedium( uintptr_t _nodePtr )
{
	m_ptrToNodes.erase( _nodePtr );
}

bool
CNodesManager ::getAddress( uintptr_t _nodePtr, CAddress & _address ) const
{
	typename std::map< uintptr_t, CNodeMedium* >::const_iterator iterator = m_ptrToNodes.find( _nodePtr );

	if ( iterator == m_ptrToNodes.end() )
		return false;

	_address = iterator->second->getNode()->addr;

	return true;
}
}
