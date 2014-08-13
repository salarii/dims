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
	CSpecificMediumFilter( long long unsigned _ptr )
	: m_ptr( _ptr )
	{}

	std::list< common::CMedium< NodeResponses > *> getMediums( client::CTrackerLocalRanking * _trackerLocalRanking )const
	{
		std::list< common::CMedium< NodeResponses > *> mediums;

		return mediums;
	}
	long long unsigned m_ptr;
};


}

#endif // CLIENT_FILTERS_H
