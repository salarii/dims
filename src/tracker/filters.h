#ifndef TRACKER_FILTERS_H
#define TRACKER_FILTERS_H

#include "common/filters.h"
#include "trackerNodesManager.h"
#include "internalMediumProvider.h"

namespace tracker
{

struct CMediumClassFilter : public common::CMediumFilter
{
	CMediumClassFilter( common::CMediumKinds::Enum _mediumClass, unsigned int _mediumNumber = (unsigned int)-1 ):
		m_mediumClass( _mediumClass ),
		m_mediumNumber( _mediumNumber )
	{}

	std::list< common::CMedium *> getMediums( common::CNodesManager * _trackerNodesManager )const
	{
		std::list< common::CMedium *> mediums;
		mediums = _trackerNodesManager->getNodesByClass( m_mediumClass );

		if ( m_mediumNumber != (unsigned int)-1 && mediums.size() > m_mediumNumber )
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
	CNodeExceptionFilter( common::CMediumKinds::Enum _mediumClass, CPubKey const & _exceptionKey )
		: m_mediumClass(_mediumClass)
	{
		m_exceptions.insert( _exceptionKey );
	}

	CNodeExceptionFilter( common::CMediumKinds::Enum _mediumClass, std::set< CPubKey > const & _exceptionKeys )
		: m_exceptions( _exceptionKeys )
		, m_mediumClass(_mediumClass)
	{
	}

	std::list< common::CMedium *> getMediums( CTrackerNodesManager * _nodesManager )const
	{
		std::list< common::CMedium *> mediums;

		mediums = _nodesManager->getNodesByClass( common::CMediumKinds::Trackers );

		BOOST_FOREACH( CPubKey const & key, m_exceptions )
		{
			uintptr_t nodeIndicator;

			_nodesManager->getKeyToNode( key.GetID(), nodeIndicator );

			common::CMedium * medium = _nodesManager->findNodeMedium( nodeIndicator );
			mediums.remove( medium );
		}

		return mediums;
	}

	std::set<CPubKey> m_exceptions;
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

struct CComplexMediumFilter : public common::CMediumFilter
{
	CComplexMediumFilter( std::set< uint160> const & _keyIds )
		: m_keyIds( _keyIds )
	{}

	std::list< common::CMedium *> getMediums( CTrackerNodesManager * _nodesManager )const
	{

		std::list< common::CMedium *> mediums;

		BOOST_FOREACH( uint160 const & keyId , m_keyIds )
		{
			uintptr_t nodeIndicator;
			_nodesManager->getKeyToNode( keyId, nodeIndicator );

			common::CMedium * medium = _nodesManager->findNodeMedium( nodeIndicator );
			if ( medium )
				mediums.push_back( medium );
		}
		return mediums;
	}
	 std::set< uint160 > m_keyIds;
};


struct CByKeyMediumFilter : public common::CMediumFilter
{
	CByKeyMediumFilter( CPubKey const & _key )
	: m_key( _key )
	{}

	std::list< common::CMedium *> getMediums( CTrackerNodesManager * _nodesManager )const
	{
		std::list< common::CMedium *> mediums;

		uintptr_t nodeIndicator;
		_nodesManager->getKeyToNode( m_key.GetID(), nodeIndicator );

		common::CMedium * medium = _nodesManager->findNodeMedium( nodeIndicator );

		if ( medium )
			mediums.push_back( medium );

		return mediums;
	}
	CPubKey m_key;
};


}

#endif // TRACKER_FILTERS_H
