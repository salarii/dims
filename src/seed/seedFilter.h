// @2011 - @2014 sipa

#ifndef SEED_FILTER_H
#define SEED_FILTER_H

#include "common/filters.h"
#include "common/mediumKinds.h"
#include "common/timeMedium.h"

#include "configureSeedActionHandler.h"

namespace seed
{

struct CSpecificMediumFilter : public common::CMediumFilter< SeedResponses >
{
	CSpecificMediumFilter( uintptr_t _ptr )
	: m_ptr( _ptr )
	{}

	std::list< common::CMedium< SeedResponses > *> getMediums( CSeedNodesManager * _nodesManager )const
	{
		std::list< common::CMedium< SeedResponses > *> mediums;

		mediums.push_back( _nodesManager->findNodeMedium( m_ptr ) );

		return mediums;
	}
	uintptr_t m_ptr;
};


struct CMediumClassFilter : public common::CMediumFilter< SeedResponses >
{
	CMediumClassFilter( common::CMediumKinds::Enum _mediumClass, int _mediumNumber = -1 ):
		m_mediumClass( _mediumClass ),
		m_mediumNumber( _mediumNumber )
	{}
//  fix  it
	std::list< common::CMedium< SeedResponses > *> getMediums( CSeedNodesManager * _nodesManager )const
	{
		if ( common::CMediumKinds::Internal == m_mediumClass )
		{
			return _nodesManager->getInternalMedium();// not nice
		}
		else if( common::CMediumKinds::Time == m_mediumClass )
		{
			std::list< common::CMedium< SeedResponses > *> mediums;
			mediums.push_back( common::CTimeMedium< SeedResponses >::getInstance() );//not nice
			return mediums;
		}
	}
	common::CMediumKinds::Enum m_mediumClass;
	int m_mediumNumber;
};

}

#endif // SEED_FILTER_H
