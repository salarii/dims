#ifndef TRACKER_FILTERS_H
#define TRACKER_FILTERS_H

#include "configureMonitorActionHandler.h"

#include "common/filters.h"
#include "common/nodesManager.h"

#include "internalMediumProvider.h"

namespace monitor
{

struct CMediumClassFilter : public common::CMonitorMediumFilter
{
	CMediumClassFilter( common::CMediumKinds::Enum _mediumClass, int _mediumNumber = ( unsigned int )-1 ):
		m_mediumClass( _mediumClass ),
		m_mediumNumber( _mediumNumber )
	{}

	std::list< common::CMonitorBaseMedium *> getMediums( common::CNodesManager< common::CMonitorTypes > * _nodesManager )const
	{
		std::list< common::CMonitorBaseMedium *> mediums;
		mediums = _nodesManager->getNodesByClass( m_mediumClass );

		if ( m_mediumNumber != ( unsigned int )-1 && mediums.size() > m_mediumNumber )
		{
			mediums.resize( m_mediumNumber );
		}
		return mediums;
	}

	std::list< common::CMonitorBaseMedium *> getMediums( CInternalMediumProvider * _internalMediumProvider )const
	{
		return _internalMediumProvider->getMediumByClass( m_mediumClass, m_mediumNumber );
	}

	common::CMediumKinds::Enum m_mediumClass;
	unsigned int m_mediumNumber;
};

struct CNodeExceptionFilter : public common::CMonitorMediumFilter
{
	CNodeExceptionFilter( uintptr_t _exception ):m_exception( _exception )
	{}

	std::list< common::CMonitorBaseMedium *> getMediums( common::CNodesManager< common::CMonitorTypes > * _trackerNodesManager )const
	{
		std::list< common::CMonitorBaseMedium *> mediums;

		mediums = _trackerNodesManager->getNodesByClass( common::CMediumKinds::Trackers );
		mediums.remove( _trackerNodesManager->findNodeMedium( m_exception ) );

		return mediums;
	}

	uintptr_t m_exception;
};

struct CSpecificMediumFilter : public common::CMonitorMediumFilter
{
	CSpecificMediumFilter( uintptr_t _ptr )
	: m_ptr( _ptr )
	{}

	std::list< common::CMonitorBaseMedium *> getMediums( common::CNodesManager< common::CMonitorTypes > * _nodesManager )const
	{
		std::list< common::CMonitorBaseMedium *> mediums;

		common::CMonitorBaseMedium * medium = _nodesManager->findNodeMedium( m_ptr );

		if ( medium )
			mediums.push_back( medium );

		return mediums;
	}
	uintptr_t m_ptr;
};

struct CComplexMediumFilter : public common::CMonitorMediumFilter
{
	CComplexMediumFilter( std::set< uintptr_t > const & _nodes )
		: m_nodes( _nodes )
	{}
	std::list< common::CMonitorBaseMedium *> getMediums( common::CNodesManager< common::CMonitorTypes > * _nodesManager )const
	{

		std::list< common::CMonitorBaseMedium *> mediums;

		BOOST_FOREACH( uintptr_t nodePtr , m_nodes )
		{
			common::CMonitorBaseMedium * medium = _nodesManager->findNodeMedium( nodePtr );

			if ( medium )
				mediums.push_back( medium );
		}
		return mediums;
	}

	 std::set< uintptr_t > const & m_nodes;
};


}

#endif // TRACKER_FILTERS_H
