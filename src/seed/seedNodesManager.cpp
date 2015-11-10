// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/actionHandler.h"

#include "seedNodesManager.h"
#include "seedNodeMedium.h"
#include "internalMedium.h"
#include "pingAction.h"

namespace common
{
std::vector< uint256 > deleteList;
}

namespace seed
{

CSeedNodesManager*
CSeedNodesManager::getInstance( )
{
	if ( !ms_instance )
	{
		ms_instance = new CSeedNodesManager();
	};
	return static_cast<CSeedNodesManager *>( ms_instance );
}


CSeedNodesManager::CSeedNodesManager()
{
}

std::list< common::CMedium *>
CSeedNodesManager::provideConnection( common::CMediumFilter const & _mediumFilter )
{
	return _mediumFilter.getMediums( this );
}

std::list< common::CMedium *>
CSeedNodesManager::getInternalMedium()
{
	std::list< common::CMedium *> mediums;

	mediums.push_back( CInternalMedium::getInstance() );

	return mediums;
}

void
CSeedNodesManager::setNodePublicKey( uintptr_t _nodeIndicator, CPubKey const & _pubKey )
{
	m_nodeKeyStore.insert( std::make_pair( _nodeIndicator, _pubKey ) );
}

bool
CSeedNodesManager::getNodePublicKey( uintptr_t _nodeIndicator, CPubKey & _pubKey ) const
{
	std::map< uintptr_t, CPubKey >::const_iterator iterator = m_nodeKeyStore.find( _nodeIndicator );

	if ( iterator == m_nodeKeyStore.end() )
		return false;

	_pubKey = iterator->second;

	return true;
}

bool
CSeedNodesManager::getKeyToNode( CPubKey const & _pubKey, uintptr_t & _nodeIndicator )
{
	BOOST_FOREACH( PAIRTYPE( uintptr_t, CPubKey ) const & node, m_nodeKeyStore )
	{
		if ( node.second == _pubKey )
		{
			_nodeIndicator = node.first;
			return true;
		}
	}

	return false;
}

bool
CSeedNodesManager::clearPublicKey( uintptr_t _nodeIndicator )
{
	m_nodeKeyStore.erase( _nodeIndicator );
	return true;
}

bool
CSeedNodesManager::isKnown( CPubKey const & _pubKey ) const
{
	std::map< uintptr_t, CPubKey >::const_iterator iterator = m_nodeKeyStore.begin();

	while( iterator != m_nodeKeyStore.end() )
	{
		if ( iterator->second == _pubKey )
			return true;
		iterator++;
	}
	return false;
}

void
CSeedNodesManager::evaluateNode( common::CSelfNode * _selfNode )
{
	common::CActionHandler::getInstance()->executeAction( new CPingAction( _selfNode ) );
}

}
