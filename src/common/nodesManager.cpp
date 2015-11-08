// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/nodesManager.h"

namespace common
{

CNodesManager * common::CNodesManager::ms_instance = 0;

CNodesManager::CNodesManager()
{
}

void
CNodesManager::addNode( CNodeMedium * _medium )
{
	boost::lock_guard<boost::mutex> lock( m_nodesLock );
// create  and  run ping  action
	m_ptrToNodes.insert( std::make_pair( convertToInt( _medium->getNode() ), _medium ) );
}

CNodeMedium*
CNodesManager::getMediumForNode( common::CSelfNode * _node ) const
{
	typename std::map< uintptr_t, CNodeMedium* >::const_iterator iterator = m_ptrToNodes.find( convertToInt( _node ) );
	if ( iterator != m_ptrToNodes.end() )
	{
		return iterator->second;
	}

	return 0;
}

std::list< CMedium*>
CNodesManager::provideConnection( CMediumFilter const & _mediumFilter )
{
	return _mediumFilter.getMediums( this );
}

 CMedium *
CNodesManager::findNodeMedium( uintptr_t _ptr ) const
{
	typename std::map< uintptr_t, CNodeMedium* >::const_iterator iterator = m_ptrToNodes.find( _ptr );

	return iterator != m_ptrToNodes.end() ? iterator->second : 0;
}

void
CNodesManager::eraseMedium( uintptr_t _nodePtr )
{
	m_ptrToNodes.erase( _nodePtr );
}

bool
CNodesManager::getAddress( uintptr_t _nodePtr, CAddress & _address ) const
{
	typename std::map< uintptr_t, CNodeMedium* >::const_iterator iterator = m_ptrToNodes.find( _nodePtr );

	if ( iterator == m_ptrToNodes.end() )
		return false;

	_address = iterator->second->getNode()->addr;

	return true;
}

void
CNodesManager::setPublicKey( CAddress const & _address, CPubKey const & _pubKey )
{
	m_keyStore.insert( std::make_pair( _address, _pubKey ) );
}

bool
CNodesManager::getPublicKey( CAddress const & _address, CPubKey & _pubKey ) const
{
	std::map< CAddress, CPubKey, CCustomAddressComparator >::const_iterator iterator = m_keyStore.find( _address );

	if ( iterator == m_keyStore.end() )
		return false;

	_pubKey = iterator->second;

	return true;
}

void
CNodesManager::erasePubKey( CAddress const & _address )
{
	m_keyStore.erase( _address );
}

}
