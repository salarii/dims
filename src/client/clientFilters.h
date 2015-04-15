// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CLIENT_FILTERS_H
#define CLIENT_FILTERS_H

#include "common/filters.h"

#include "client/configureClientActionHadler.h"
#include "client/settingsConnectionProvider.h"
#include "client/trackerLocalRanking.h"
#include "client/applicationServer.h"
#include "client/errorMediumProvider.h"
#include "client/settingsMedium.h"

namespace  client
{

struct CMediumClassFilter : public common::CClientMediumFilter
{
	CMediumClassFilter( common::RequestKind::Enum _mediumClass, int _mediumNumber = ( unsigned int )-1 ):
		m_mediumClass( _mediumClass ),
		m_mediumNumber( _mediumNumber )
	{}

	std::list< common::CClientBaseMedium *> getMediums( CSettingsConnectionProvider * _settingsMedium )const
	{

		std::list< common::CClientBaseMedium *> mediums;
		mediums = _settingsMedium->getMediumByClass(m_mediumClass );

		if ( m_mediumNumber != (unsigned int)-1 && mediums.size() > m_mediumNumber )
		{
			mediums.resize( m_mediumNumber );
		}
		return mediums;
	}

	std::list< common::CClientBaseMedium *> getMediums( client::CTrackerLocalRanking * _trackerLocalRanking )const
	{
		std::list< common::CClientBaseMedium *> mediums = _trackerLocalRanking->getMediumByClass( m_mediumClass, m_mediumNumber );

		if ( m_mediumNumber != ( unsigned int )-1 && mediums.size() > m_mediumNumber )
		{
			mediums.resize( m_mediumNumber );
		}

		return mediums;
	}

	std::list< common::CClientBaseMedium *> getMediums( CErrorMediumProvider * _errorMediumProvider )const
	{
		return _errorMediumProvider->getErrorMedium();
	}

	common::RequestKind::Enum m_mediumClass;
	unsigned int m_mediumNumber;
};

struct CSpecificMediumFilter : public common::CClientMediumFilter
{
	CSpecificMediumFilter( std::set< uintptr_t > const & _nodes )
		: m_nodes( _nodes )
	{}

	CSpecificMediumFilter( uintptr_t const _node )
	{
		m_nodes.insert( _node );
	}

	std::list< common::CClientBaseMedium *> getMediums( client::CTrackerLocalRanking * _trackerLocalRanking )const
	{

		std::list< common::CClientBaseMedium *> mediums;

		BOOST_FOREACH( uintptr_t nodePtr , m_nodes )
		{
			common::CClientBaseMedium * medium = _trackerLocalRanking->getSpecificTracker( nodePtr );
			if ( medium )
				mediums.push_back( medium );
		}

		return mediums;
	}

	std::list< common::CClientBaseMedium *> getMediums( CLocalServer * _localServer )const
	{

		std::list< common::CClientBaseMedium *> mediums;

		CLocalSocket * localSocket;

		BOOST_FOREACH( uintptr_t nodePtr , m_nodes )
		{
			if ( CLocalServer::getInstance()->getSocked( nodePtr, localSocket ) )
				mediums.push_back( localSocket );
		}
		return mediums;
	}

	std::list< common::CClientBaseMedium *> getMediums( CErrorMediumProvider * _errorMediumProvider )const
	{
		return _errorMediumProvider->getErrorMedium();
	}

	 std::set< uintptr_t > m_nodes;
};

struct CMediumByKeyFilter : public common::CClientMediumFilter
{
	CMediumByKeyFilter( CKeyID const & _keyId )
		: m_keyId( _keyId )
	{}

	std::list< common::CClientBaseMedium *> getMediums( client::CTrackerLocalRanking * _trackerLocalRanking )const
	{

		std::list< common::CClientBaseMedium *> mediums;

		common::CClientBaseMedium * medium;

		if ( _trackerLocalRanking->getSpecificMedium( m_keyId, medium ) )
			mediums.push_back( medium );

		return mediums;
	}

	std::list< common::CClientBaseMedium *> getMediums( CErrorMediumProvider * _errorMediumProvider )const
	{
		return _errorMediumProvider->getErrorMedium();
	}
	CKeyID const m_keyId;
};

struct CMediumClassWithExceptionFilter : public common::CClientMediumFilter
{
	CMediumClassWithExceptionFilter( uintptr_t const & _exceptionPtr, int _mediumClass, int _mediumNumber = -1 )
		: m_mediumClass( _mediumClass )
		, m_mediumNumber( _mediumNumber )
	{
		m_exceptionPtrs.insert( _exceptionPtr );
	}

	CMediumClassWithExceptionFilter( std::set< uintptr_t > const & _exceptionPtrs, int _mediumClass, int _mediumNumber = -1 )
		: m_mediumClass( _mediumClass )
		, m_exceptionPtrs( _exceptionPtrs )
		, m_mediumNumber( _mediumNumber )
	{}


	std::list< common::CClientBaseMedium *> getMediums( client::CTrackerLocalRanking * _trackerLocalRanking )const
	{
		std::list< common::CClientBaseMedium *> mediums;
		mediums = _trackerLocalRanking->getMediumByClass( ( common::RequestKind::Enum )m_mediumClass, (unsigned int)-1 );

		BOOST_FOREACH( uintptr_t const & ptr, m_exceptionPtrs )
		{
			common::CClientBaseMedium * medium = _trackerLocalRanking->getSpecificTracker( ptr );

			std::list< common::CClientBaseMedium *>::iterator iterator = std::find( mediums.begin(), mediums.end(), medium );

			if ( iterator != mediums.end() )
			{
				mediums.erase( iterator );
			}

		}

		if ( m_mediumNumber != (unsigned int)-1 && mediums.size() > m_mediumNumber )
		{
			mediums.resize( m_mediumNumber );
		}

		return mediums;
	}

	std::list< common::CClientBaseMedium *> getMediums( CErrorMediumProvider * _errorMediumProvider )const
	{
		return _errorMediumProvider->getErrorMedium();
	}

	int m_mediumClass;
	std::set< uintptr_t > m_exceptionPtrs;
	unsigned int m_mediumNumber;
};

}

#endif // CLIENT_FILTERS_H
