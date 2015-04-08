// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "seedNodesManager.h"
#include "seedNodeMedium.h"
#include "internalMedium.h"

namespace common
{
std::vector< uint256 > deleteList;

template<>	CNodesManager< common::CSeedTypes > * common::CNodesManager< common::CSeedTypes >::ms_instance = 0;
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

std::list< common::CSeedBaseMedium *>
CSeedNodesManager::provideConnection( common::CSeedMediumFilter const & _mediumFilter )
{
	return _mediumFilter.getMediums( this );
}

std::list< common::CSeedBaseMedium *>
CSeedNodesManager::getInternalMedium()
{
	std::list< common::CSeedBaseMedium *> mediums;

	mediums.push_back( CInternalMedium::getInstance() );

	return mediums;
}

void
CSeedNodesManager::setPublicKey( uintptr_t _nodeIndicator, CPubKey const & _pubKey )
{
	m_keyStore.insert( std::make_pair( _nodeIndicator, _pubKey ) );
}

bool
CSeedNodesManager::getPublicKey( uintptr_t _nodeIndicator, CPubKey & _pubKey ) const
{
	std::map< uintptr_t, CPubKey >::const_iterator iterator = m_keyStore.find( _nodeIndicator );

	if ( iterator == m_keyStore.end() )
		return false;

	_pubKey = iterator->second;

	return true;
}

bool
CSeedNodesManager::clearPublicKey( uintptr_t _nodeIndicator )
{
	m_keyStore.erase( _nodeIndicator );
}

bool
CSeedNodesManager::isKnown( CPubKey const & _pubKey ) const
{
	std::map< uintptr_t, CPubKey >::const_iterator iterator = m_keyStore.begin();

	while( iterator != m_keyStore.end() )
	{
		if ( iterator->second == _pubKey )
			return true;
		iterator++;
	}
	return false;
}

}
