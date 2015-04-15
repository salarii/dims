#ifndef TRACKER_FILTERS_H
#define TRACKER_FILTERS_H

#include "common/filters.h"
#include "trackerNodesManager.h"
#include "internalMediumProvider.h"

namespace tracker
{

struct CMediumClassFilter : public common::CTrackerMediumFilter
{
	CMediumClassFilter( common::CMediumKinds::Enum _mediumClass, unsigned int _mediumNumber = (unsigned int)-1 ):
		m_mediumClass( _mediumClass ),
		m_mediumNumber( _mediumNumber )
	{}

	std::list< common::CTrackerBaseMedium *> getMediums( common::CNodesManager< common::CTrackerTypes > * _trackerNodesManager )const
	{
		std::list< common::CTrackerBaseMedium *> mediums;
		mediums = _trackerNodesManager->getNodesByClass( m_mediumClass );

		if ( m_mediumNumber != (unsigned int)-1 && mediums.size() > m_mediumNumber )
		{
			mediums.resize( m_mediumNumber );
		}
		return mediums;
	}

	std::list< common::CTrackerBaseMedium *> getMediums( CInternalMediumProvider * _internalMediumProvider )const
	{
		return _internalMediumProvider->getMediumByClass( m_mediumClass, m_mediumNumber );
	}

	common::CMediumKinds::Enum m_mediumClass;
	unsigned int m_mediumNumber;
};

struct CNodeExceptionFilter : public common::CTrackerMediumFilter
{
	CNodeExceptionFilter( uintptr_t _exception ):m_exception( _exception )
	{}

	std::list< common::CTrackerBaseMedium *> getMediums( common::CNodesManager< common::CTrackerTypes > * _trackerNodesManager )const
	{
		std::list< common::CTrackerBaseMedium *> mediums;

		mediums = _trackerNodesManager->getNodesByClass( common::CMediumKinds::Trackers );
		mediums.remove( _trackerNodesManager->findNodeMedium( m_exception ) );

		return mediums;
	}

	uintptr_t m_exception;
};

struct CSpecificMediumFilter : public common::CTrackerMediumFilter
{
	CSpecificMediumFilter( uintptr_t _ptr )
	: m_ptr( _ptr )
	{}

	std::list< common::CTrackerBaseMedium *> getMediums( common::CNodesManager< common::CTrackerTypes > * _nodesManager )const
	{
		std::list< common::CTrackerBaseMedium *> mediums;

		mediums.push_back( _nodesManager->findNodeMedium( m_ptr ) );

		return mediums;
	}
	uintptr_t m_ptr;
};

struct CComplexMediumFilter : public common::CTrackerMediumFilter
{
	CComplexMediumFilter( std::set< uintptr_t > const & _nodes )
		: m_nodes( _nodes )
	{}

	std::list< common::CTrackerBaseMedium *> getMediums( common::CNodesManager< common::CTrackerTypes > * _nodesManager )const
	{

		std::list< common::CTrackerBaseMedium *> mediums;

		BOOST_FOREACH( uintptr_t nodePtr , m_nodes )
		{
			common::CTrackerBaseMedium * medium = _nodesManager->findNodeMedium( nodePtr );
			if ( medium )
				mediums.push_back( medium );
		}
		return mediums;
	}
	 std::set< uintptr_t > const & m_nodes;
};


}

#endif // TRACKER_FILTERS_H
