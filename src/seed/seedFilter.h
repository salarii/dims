// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SEED_FILTER_H
#define SEED_FILTER_H

#include "common/filters.h"
#include "common/mediumKinds.h"
#include "common/timeMedium.h"


#include "seed/seedNodesManager.h"

namespace seed
{

struct CSpecificMediumFilter : public common::CMediumFilter
{
	CSpecificMediumFilter( uintptr_t _ptr )
	: m_ptr( _ptr )
	{}

	std::list< common::CMedium *> getMediums( CSeedNodesManager * _nodesManager )const
	{
		std::list< common::CMedium *> mediums;

		mediums.push_back( _nodesManager->findNodeMedium( m_ptr ) );

		return mediums;
	}
	uintptr_t m_ptr;
};


struct CMediumClassFilter : public common::CMediumFilter
{
	CMediumClassFilter( common::CMediumKinds::Enum _mediumClass, int _mediumNumber = -1 ):
		m_mediumClass( _mediumClass ),
		m_mediumNumber( _mediumNumber )
	{}
//  fix  it
	std::list< common::CMedium *> getMediums( CSeedNodesManager * _nodesManager )const
	{
		std::list< common::CMedium *> mediums;
		if ( common::CMediumKinds::Internal == m_mediumClass )
		{
			return _nodesManager->getInternalMedium();// not nice
		}
		else if( common::CMediumKinds::Time == m_mediumClass )
		{
			mediums.push_back( common::CTimeMedium::getInstance() );//not nice
		}
		return mediums;
	}
	common::CMediumKinds::Enum m_mediumClass;
	int m_mediumNumber;
};

struct CByKeyMediumFilter : public common::CMediumFilter
{
	CByKeyMediumFilter( CPubKey const & _key )
	: m_key( _key )
	{}

	std::list< common::CMedium *> getMediums( CSeedNodesManager * _nodesManager )const
	{
		std::list< common::CMedium *> mediums;

		uintptr_t nodeIndicator;
		_nodesManager->getKeyToNode( m_key, nodeIndicator );

		common::CMedium * medium = _nodesManager->findNodeMedium( nodeIndicator );

		if ( medium )
			mediums.push_back( medium );

		return mediums;
	}
	CPubKey m_key;
};

}

#endif // SEED_FILTER_H
