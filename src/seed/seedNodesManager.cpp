// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "seedNodesManager.h"
#include "seedNodeMedium.h"
#include "internalMedium.h"

namespace common
{
std::vector< uint256 > deleteList;

template<>	CNodesManager< seed::SeedResponses > * common::CNodesManager< seed::SeedResponses >::ms_instance = 0;
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

CSeedNodeMedium*
CSeedNodesManager::getMediumForNode( common::CSelfNode * _node ) const
{
	return static_cast< CSeedNodeMedium* >( common::CNodesManager< SeedResponses >::getMediumForNode( _node ) );
}

std::list< common::CMedium< SeedResponses > *>
CSeedNodesManager::provideConnection( int const _actionKind, unsigned _requestedConnectionNumber )
{
	std::list< common::CMedium< SeedResponses > *> mediums = common::CNodesManager< SeedResponses >::provideConnection( _actionKind, _requestedConnectionNumber );

	if ( !mediums.empty() )
		return mediums;

	if ( !_actionKind )// not  correct
	{
		mediums.push_back( CInternalMedium::getInstance() );
	}
	return mediums;
}

bool
CSeedNodesManager::getKeyForNode( common::CSelfNode * _node, CPubKey & _key ) const
{
	if ( m_keyStore.find( _node ) != m_keyStore.end() )
	{
		_key = m_keyStore.find( _node )->second;
		return true;
	}
	else
		return false;
}

}
