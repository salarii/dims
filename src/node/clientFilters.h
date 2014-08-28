#ifndef CLIENT_FILTERS_H
#define CLIENT_FILTERS_H

#include "common/filters.h"
#include "configureNodeActionHadler.h"
#include "settingsConnectionProvider.h"
#include "trackerLocalRanking.h"

namespace  client
{

struct CMediumClassFilter : public common::CMediumFilter< NodeResponses >
{
	CMediumClassFilter( int _mediumClass, int _mediumNumber = -1 ):
		m_mediumClass( _mediumClass ),
		m_mediumNumber( _mediumNumber )
	{}

	std::list< common::CMedium< NodeResponses > *> getMediums( CSettingsConnectionProvider * _settingsMedium )const
	{

		std::list< common::CMedium< NodeResponses > *> mediums;
		mediums = _settingsMedium->getMediumByClass( ( common::RequestKind::Enum )m_mediumClass );

		if ( m_mediumNumber != -1 && mediums.size() > m_mediumNumber )
		{
			mediums.resize( m_mediumNumber );
		}
		return mediums;
	}

	std::list< common::CMedium< NodeResponses > *> getMediums( client::CTrackerLocalRanking * _trackerLocalRanking )const
	{
		return _trackerLocalRanking->getMediumByClass( ( common::RequestKind::Enum )m_mediumClass, m_mediumNumber );
	}

	int m_mediumClass;
	int m_mediumNumber;
};

struct CSpecificMediumFilter : public common::CMediumFilter< NodeResponses >
{
	CSpecificMediumFilter( std::set< uintptr_t > const & _nodes )
		: m_nodes( _nodes )
	{}

	CSpecificMediumFilter( uintptr_t const _node )
	{
		m_nodes.insert( _node );
	}

	std::list< common::CMedium< NodeResponses > *> getMediums( client::CTrackerLocalRanking * _trackerLocalRanking )const
	{

		std::list< common::CMedium< NodeResponses > *> mediums;

		BOOST_FOREACH( uintptr_t nodePtr , m_nodes )
		{
			common::CMedium< NodeResponses > * medium = _trackerLocalRanking->getSpecificTracker( nodePtr );
			if ( medium )
				mediums.push_back( medium );
		}
		return mediums;
	}
	 std::set< uintptr_t > m_nodes;
};

struct CMediumClassWithExceptionFilter : public common::CMediumFilter< NodeResponses >
{
	CMediumClassWithExceptionFilter( int _mediumClass, uintptr_t const & _exceptionPtr, int _mediumNumber = -1 )
		: m_mediumClass( _mediumClass ),
		  m_exceptionPtr( _exceptionPtr ),
		  m_mediumNumber( _mediumNumber )
	{}

	std::list< common::CMedium< NodeResponses > *> getMediums( client::CTrackerLocalRanking * _trackerLocalRanking )const
	{
		std::list< common::CMedium< NodeResponses > *> mediums;
		mediums = _trackerLocalRanking->getMediumByClass( ( common::RequestKind::Enum )m_mediumClass, -1 );

		common::CMedium< NodeResponses > * medium = _trackerLocalRanking->getSpecificTracker( m_exceptionPtr );

		if ( mediums.size() <= 1 )
			return mediums;

		std::list< common::CMedium< NodeResponses > *>::iterator iterator = std::find( mediums.begin(), mediums.end(), medium );

		if ( iterator != mediums.end() )
		{
			mediums.erase( iterator );
		}

		if ( m_mediumNumber != -1 && mediums.size() > m_mediumNumber )
		{
			mediums.resize( m_mediumNumber );
		}

		return mediums;
	}
	int m_mediumClass;
	uintptr_t m_exceptionPtr;
	int m_mediumNumber;
};

}

#endif // CLIENT_FILTERS_H
