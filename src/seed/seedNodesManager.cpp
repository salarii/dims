// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "seedNodesManager.h"
#include "seedNodeMedium.h"
#include "internalMedium.h"

namespace common
{
std::vector< uint256 > deleteList;

template<>	CNodesManager< common::CSeedMediumFilter > * common::CNodesManager< common::CSeedMediumFilter >::ms_instance = 0;
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
CSeedNodesManager::setPublicKey( CAddress const & _address, CPubKey const & _pubKey )
{
	m_keyStore.insert( std::make_pair( _address, _pubKey ) );
}

bool
CSeedNodesManager::getPublicKey( CAddress const & _address, CPubKey & _pubKey ) const
{
	std::map< CAddress, CPubKey >::const_iterator iterator = m_keyStore.find( _address );

	if ( iterator == m_keyStore.end() )
		return false;

	_pubKey = iterator->second;

	return true;
}


}
