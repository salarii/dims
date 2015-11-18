#ifndef TRACKER_FILTERS_H
#define TRACKER_FILTERS_H

#include "common/filters.h"
#include "common/nodesManager.h"

#include "monitor/internalMediumProvider.h"
#include "monitor/reputationTracer.h"

namespace monitor
{

struct CMediumClassFilter : public common::CMediumFilter
{
	CMediumClassFilter( common::CMediumKinds::Enum _mediumClass, int _mediumNumber = ( unsigned int )-1 ):
		m_mediumClass( _mediumClass ),
		m_mediumNumber( _mediumNumber )
	{}

	std::list< common::CMedium *> getMediums( CReputationTracker * _nodesManager )const
	{
		std::list< common::CMedium *> mediums;
		mediums = _nodesManager->getNodesByClass( m_mediumClass );

		if ( m_mediumNumber != ( unsigned int )-1 && mediums.size() > m_mediumNumber )
		{
			mediums.resize( m_mediumNumber );
		}
		return mediums;
	}

	std::list< common::CMedium *> getMediums( CInternalMediumProvider * _internalMediumProvider )const
	{
		return _internalMediumProvider->getMediumByClass( m_mediumClass, m_mediumNumber );
	}

	common::CMediumKinds::Enum m_mediumClass;
	unsigned int m_mediumNumber;
};

struct CNodeExceptionFilter : public common::CMediumFilter
{
	CNodeExceptionFilter( common::CMediumKinds::Enum _mediumClass, uint160 const & _exceptionKeyId )
		: m_mediumClass(_mediumClass)
	{
		m_exceptions.insert( _exceptionKeyId );
	}

	CNodeExceptionFilter( common::CMediumKinds::Enum _mediumClass, std::set< uint160 > const & _exceptionKeyIds )
		: m_exceptions( _exceptionKeyIds )
		, m_mediumClass(_mediumClass)
	{
	}

	std::list< common::CMedium *> getMediums( CReputationTracker * _trackerNodesManager )const
	{
		std::list< common::CMedium *> mediums;

		mediums = _trackerNodesManager->getNodesByClass( m_mediumClass );

		BOOST_FOREACH( CKeyID const & keyId, m_exceptions )
		{
			uintptr_t nodeIndicator;

			_trackerNodesManager->getKeyToNode( keyId, nodeIndicator );

			common::CMedium * medium = _trackerNodesManager->findNodeMedium( nodeIndicator );
			mediums.remove( medium );
		}

		return mediums;
	}

	std::set<uint160> m_exceptions;
	common::CMediumKinds::Enum m_mediumClass;
};

struct CSpecificMediumFilter : public common::CMediumFilter
{
	CSpecificMediumFilter( uintptr_t _ptr )
	: m_ptr( _ptr )
	{}

	std::list< common::CMedium *> getMediums( common::CNodesManager * _nodesManager )const
	{
		std::list< common::CMedium *> mediums;

		common::CMedium * medium = _nodesManager->findNodeMedium( m_ptr );

		if ( medium )
			mediums.push_back( medium );

		return mediums;
	}
	uintptr_t m_ptr;
};

struct CByKeyMediumFilter : public common::CMediumFilter
{
	CByKeyMediumFilter( CPubKey const & _key )
	: m_key( _key )
	{}

	std::list< common::CMedium *> getMediums( CReputationTracker * _nodesManager )const
	{
		std::list< common::CMedium *> mediums;

		uintptr_t nodeIndicator;
		if ( !_nodesManager->getKeyToNode( m_key.GetID(), nodeIndicator ) )
			return mediums;

		common::CMedium * medium = _nodesManager->findNodeMedium( nodeIndicator );

		if ( medium )
			mediums.push_back( medium );

		return mediums;
	}
	CPubKey m_key;
};

struct CComplexMediumFilter : public common::CMediumFilter
{
	CComplexMediumFilter( std::set< uintptr_t > const & _nodes )
		: m_nodes( _nodes )
	{}
	std::list< common::CMedium *> getMediums( common::CNodesManager * _nodesManager )const
	{

		std::list< common::CMedium *> mediums;

		BOOST_FOREACH( uintptr_t nodePtr , m_nodes )
		{
			common::CMedium * medium = _nodesManager->findNodeMedium( nodePtr );

			if ( medium )
				mediums.push_back( medium );
		}
		return mediums;
	}

	 std::set< uintptr_t > const & m_nodes;
};


}

#endif // TRACKER_FILTERS_H
