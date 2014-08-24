#ifndef TRACKER_FILTERS_H
#define TRACKER_FILTERS_H

#include "common/filters.h"
#include "trackerNodesManager.h"
#include "internalMediumProvider.h"

namespace tracker
{

struct CMediumClassFilter : public common::CMediumFilter< TrackerResponses >
{
	CMediumClassFilter( int _mediumClass, int _mediumNumber = -1 ):
		m_mediumClass( _mediumClass ),
		m_mediumNumber( _mediumNumber )
	{}

	std::list< common::CMedium< TrackerResponses > *> getMediums( common::CNodesManager< TrackerResponses > * _trackerNodesManager )const
	{
		std::list< common::CMedium< TrackerResponses > *> mediums;
		mediums = _trackerNodesManager->getNodesByClass( ( common::CMediumKinds::Enum )m_mediumClass );

		if ( m_mediumNumber != -1 && mediums.size() > m_mediumNumber )
		{
			mediums.resize( m_mediumNumber );
		}
		return mediums;
	}

	std::list< common::CMedium< TrackerResponses > *> getMediums( CInternalMediumProvider * _internalMediumProvider )const
	{
		return _internalMediumProvider->getMediumByClass( ( common::CMediumKinds::Enum )m_mediumClass, m_mediumNumber );
	}

	int m_mediumClass;
	int m_mediumNumber;
};

/*
getting given medium, referencing it by pointer, is problematic in some cases ( multiple inheritance ) it is also unaesthetic and bad  practice.
maybe at some point replace it by keyID??


*/

struct CSpecificMediumFilter : public common::CMediumFilter< TrackerResponses >
{
	CSpecificMediumFilter( uintptr_t _ptr )
	: m_ptr( _ptr )
	{}

	std::list< common::CMedium< TrackerResponses > *> getMediums( common::CNodesManager< TrackerResponses > * _nodesManager )const
	{
		std::list< common::CMedium< TrackerResponses > *> mediums;

		mediums.push_back( _nodesManager->findNodeMedium( m_ptr ) );

		return mediums;
	}
	uintptr_t m_ptr;
};

}

#endif // TRACKER_FILTERS_H
