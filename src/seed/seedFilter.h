// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SEED_FILTER_H
#define SEED_FILTER_H

#include "common/filters.h"
#include "common/mediumKinds.h"
#include "common/timeMedium.h"

#include "configureSeedActionHandler.h"

#include "seed/seedNodesManager.h"

namespace seed
{

struct CSpecificMediumFilter : public common::CSeedMediumFilter
{
	CSpecificMediumFilter( uintptr_t _ptr )
	: m_ptr( _ptr )
	{}

	std::list< common::CSeedBaseMedium *> getMediums( CSeedNodesManager * _nodesManager )const
	{
		std::list< common::CSeedBaseMedium *> mediums;

		mediums.push_back( _nodesManager->findNodeMedium( m_ptr ) );

		return mediums;
	}
	uintptr_t m_ptr;
};


struct CMediumClassFilter : public common::CSeedMediumFilter
{
	CMediumClassFilter( common::CMediumKinds::Enum _mediumClass, int _mediumNumber = -1 ):
		m_mediumClass( _mediumClass ),
		m_mediumNumber( _mediumNumber )
	{}
//  fix  it
	std::list< common::CSeedBaseMedium *> getMediums( CSeedNodesManager * _nodesManager )const
	{
		std::list< common::CSeedBaseMedium *> mediums;
		if ( common::CMediumKinds::Internal == m_mediumClass )
		{
			return _nodesManager->getInternalMedium();// not nice
		}
		else if( common::CMediumKinds::Time == m_mediumClass )
		{
			mediums.push_back( common::CTimeMedium< common::CSeedBaseMedium >::getInstance() );//not nice
		}
		return mediums;
	}
	common::CMediumKinds::Enum m_mediumClass;
	int m_mediumNumber;
};

}

#endif // SEED_FILTER_H
