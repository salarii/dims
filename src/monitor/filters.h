#ifndef TRACKER_FILTERS_H
#define TRACKER_FILTERS_H

#include "configureMonitorActionHandler.h"

#include "common/filters.h"

namespace monitor
{

struct CMediumClassFilter : public common::CMediumFilter< MonitorResponses >
{
	CMediumClassFilter( int _mediumClass, int _mediumNumber = -1 ):
		m_mediumClass( _mediumClass ),
		m_mediumNumber( _mediumNumber )
	{}
/*
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
*/
	int m_mediumClass;
	int m_mediumNumber;
};

struct CNodeExceptionFilter : public common::CMediumFilter< MonitorResponses >
{
	CNodeExceptionFilter( uintptr_t _exception ):m_exception( _exception )
	{}
/*
	std::list< common::CMedium< TrackerResponses > *> getMediums( common::CNodesManager< TrackerResponses > * _trackerNodesManager )const
	{
		std::list< common::CMedium< TrackerResponses > *> mediums;

		mediums = _trackerNodesManager->getNodesByClass( common::CMediumKinds::Trackers );
		mediums.remove( _trackerNodesManager->findNodeMedium( m_exception ) );

		return mediums;
	}
*/
	uintptr_t m_exception;
};

struct CSpecificMediumFilter : public common::CMediumFilter< MonitorResponses >
{
	CSpecificMediumFilter( uintptr_t _ptr )
	: m_ptr( _ptr )
	{}
/*
	std::list< common::CMedium< TrackerResponses > *> getMediums( common::CNodesManager< TrackerResponses > * _nodesManager )const
	{
		std::list< common::CMedium< TrackerResponses > *> mediums;

		mediums.push_back( _nodesManager->findNodeMedium( m_ptr ) );

		return mediums;
	}

*/
	uintptr_t m_ptr;
};

struct CComplexMediumFilter : public common::CMediumFilter< MonitorResponses >
{
	CComplexMediumFilter( std::set< uintptr_t > const & _nodes )
		: m_nodes( _nodes )
	{}
/*
	std::list< common::CMedium< TrackerResponses > *> getMediums( common::CNodesManager< TrackerResponses > * _nodesManager )const
	{

		std::list< common::CMedium< TrackerResponses > *> mediums;

		BOOST_FOREACH( uintptr_t nodePtr , m_nodes )
		{
			common::CMedium< TrackerResponses > * medium = _nodesManager->findNodeMedium( nodePtr );
			if ( medium )
				mediums.push_back( medium );
		}
		return mediums;
	}
	*/
	 std::set< uintptr_t > const & m_nodes;
};


}

#endif // TRACKER_FILTERS_H
