// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CLIENT_FILTERS_H
#define CLIENT_FILTERS_H

#include "common/filters.h"
#include "configureClientActionHadler.h"
#include "settingsConnectionProvider.h"
#include "trackerLocalRanking.h"
#include "applicationServer.h"

#include "settingsMedium.h"

namespace  client
{

struct CMediumClassFilter : public common::CMediumFilter< ClientResponses >
{
	CMediumClassFilter( common::RequestKind::Enum _mediumClass, int _mediumNumber = -1 ):
		m_mediumClass( _mediumClass ),
		m_mediumNumber( _mediumNumber )
	{}

	std::list< common::CMedium< ClientResponses > *> getMediums( CSettingsConnectionProvider * _settingsMedium )const
	{

		std::list< common::CMedium< ClientResponses > *> mediums;
		mediums = _settingsMedium->getMediumByClass(m_mediumClass );

		if ( m_mediumNumber != -1 && mediums.size() > m_mediumNumber )
		{
			mediums.resize( m_mediumNumber );
		}
		return mediums;
	}

	std::list< common::CMedium< ClientResponses > *> getMediums( client::CTrackerLocalRanking * _trackerLocalRanking )const
	{
		std::list< common::CMedium< ClientResponses > *> mediums = _trackerLocalRanking->getMediumByClass( m_mediumClass, m_mediumNumber );

		if ( mediums.empty() )
		{
			mediums.push_back( CDefaultMedium::getInstance()->getInstance() );
			return mediums;
		}

		if ( m_mediumNumber != -1 && mediums.size() > m_mediumNumber )
		{
			mediums.resize( m_mediumNumber );
		}

		return mediums;
	}

	common::RequestKind::Enum m_mediumClass;
	int m_mediumNumber;
};

struct CSpecificMediumFilter : public common::CMediumFilter< ClientResponses >
{
	CSpecificMediumFilter( std::set< uintptr_t > const & _nodes )
		: m_nodes( _nodes )
	{}

	CSpecificMediumFilter( uintptr_t const _node )
	{
		m_nodes.insert( _node );
	}

	std::list< common::CMedium< ClientResponses > *> getMediums( client::CTrackerLocalRanking * _trackerLocalRanking )const
	{

		std::list< common::CMedium< ClientResponses > *> mediums;

		BOOST_FOREACH( uintptr_t nodePtr , m_nodes )
		{
			common::CMedium< ClientResponses > * medium = _trackerLocalRanking->getSpecificTracker( nodePtr );
			if ( medium )
				mediums.push_back( medium );
		}

		if ( mediums.empty() )
		{
			mediums.push_back( CDefaultMedium::getInstance()->getInstance() );
			return mediums;
		}

		return mediums;
	}

	std::list< common::CMedium< ClientResponses > *> getMediums( CLocalServer * _localServer )const
	{

		std::list< common::CMedium< ClientResponses > *> mediums;

		CLocalSocket * localSocket;

		BOOST_FOREACH( uintptr_t nodePtr , m_nodes )
		{
			if ( CLocalServer::getInstance()->getSocked( nodePtr, localSocket ) )
				mediums.push_back( localSocket );
		}
		return mediums;
	}


	 std::set< uintptr_t > m_nodes;
};

struct CMediumByKeyFilter : public common::CMediumFilter< ClientResponses >
{
	CMediumByKeyFilter( CKeyID const & _keyId )
		: m_keyId( _keyId )
	{}

	std::list< common::CMedium< ClientResponses > *> getMediums( client::CTrackerLocalRanking * _trackerLocalRanking )const
	{

		std::list< common::CMedium< ClientResponses > *> mediums;

		common::CMedium< ClientResponses > * medium;

		if ( _trackerLocalRanking->getSpecificMedium( m_keyId, medium ) )
			mediums.push_back( medium );

		if ( mediums.empty() )
		{
			mediums.push_back( CDefaultMedium::getInstance()->getInstance() );
			return mediums;
		}

		return mediums;
	}

	CKeyID const m_keyId;
};

struct CMediumClassWithExceptionFilter : public common::CMediumFilter< ClientResponses >
{
	CMediumClassWithExceptionFilter( uintptr_t const & _exceptionPtr, int _mediumClass, int _mediumNumber = -1 )
		: m_mediumClass( _mediumClass )
		, m_mediumNumber( _mediumNumber )
	{
		m_exceptionPtrs.insert( _exceptionPtr );
	}

	CMediumClassWithExceptionFilter( std::set< uintptr_t > const & _exceptionPtrs, int _mediumClass, int _mediumNumber = -1 )
		: m_exceptionPtrs( _exceptionPtrs )
		, m_mediumClass( _mediumClass )
		, m_mediumNumber( _mediumNumber )
	{}


	std::list< common::CMedium< ClientResponses > *> getMediums( client::CTrackerLocalRanking * _trackerLocalRanking )const
	{
		std::list< common::CMedium< ClientResponses > *> mediums;
		mediums = _trackerLocalRanking->getMediumByClass( ( common::RequestKind::Enum )m_mediumClass, -1 );

		BOOST_FOREACH( uintptr_t const & ptr, m_exceptionPtrs )
		{
			common::CMedium< ClientResponses > * medium = _trackerLocalRanking->getSpecificTracker( ptr );

			std::list< common::CMedium< ClientResponses > *>::iterator iterator = std::find( mediums.begin(), mediums.end(), medium );

			if ( iterator != mediums.end() )
			{
				mediums.erase( iterator );
			}

		}
		if ( mediums.empty() )
		{
			mediums.push_back( CDefaultMedium::getInstance()->getInstance() );
			return mediums;
		}

		if ( m_mediumNumber != -1 && mediums.size() > m_mediumNumber )
		{
			mediums.resize( m_mediumNumber );
		}

		return mediums;
	}
	int m_mediumClass;
	std::set< uintptr_t > m_exceptionPtrs;
	int m_mediumNumber;
};

}

#endif // CLIENT_FILTERS_H
