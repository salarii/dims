// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "errorMediumProvider.h"

#include "clientFilters.h"

namespace client
{

class CErrorMedium : public common::CMedium< ClientResponses >
{
public:

	static CErrorMedium * getInstance();

	bool serviced() const;

	void add( common::CRequest< ClientResponses > const * _request ){};

	void add( CMonitorInfoRequest const * _request )
	{
		m_requestResponse.insert( std::make_pair( ( common::CRequest< ClientResponses > * )_request, common::CNoMedium() ) );
	}

	void add( CTrackersInfoRequest const * _request )
	{
		m_requestResponse.insert( std::make_pair( ( common::CRequest< ClientResponses > * )_request, common::CNoMedium() ) );
	}

	void add( CBalanceRequest const * _request )
	{
		m_requestResponse.insert( std::make_pair( ( common::CRequest< ClientResponses > * )_request, common::CNoMedium() ) );
	}

	bool flush(){};

	bool getResponseAndClear( std::multimap< common::CRequest< ClientResponses >const*, ClientResponses > & _requestResponse );
private:
	void clearResponses();

	CErrorMedium(){};

	void getSeedIps( vector<CAddress> & _vAdd );
private:
	static CErrorMedium * ms_instance;

	std::multimap< common::CRequest< ClientResponses >const*, ClientResponses > m_requestResponse;
};


CErrorMedium * CErrorMedium::ms_instance = NULL;

CErrorMedium*
CErrorMedium::getInstance( )
{
	if ( !ms_instance )
	{
		ms_instance = new CErrorMedium();
	};
	return ms_instance;
}

bool
CErrorMedium::serviced() const
{
	return !m_requestResponse.empty();
}

bool
CErrorMedium::getResponseAndClear( std::multimap< common::CRequest< ClientResponses >const*, ClientResponses > & _requestResponse )
{
	_requestResponse = m_requestResponse;

	clearResponses();
}

void
CErrorMedium::clearResponses()
{
	m_requestResponse.clear();
}

CErrorMediumProvider * CErrorMediumProvider::ms_instance = NULL;

CErrorMediumProvider*
CErrorMediumProvider::getInstance()
{
	if ( !ms_instance )
	{
		ms_instance = new CErrorMediumProvider();
	};
	return ms_instance;
}

CErrorMediumProvider::CErrorMediumProvider()
{
}

std::list< common::CMedium< ClientResponses > *>
CErrorMediumProvider::provideConnection( common::CMediumFilter< ClientResponses > const & _mediumFilter )
{
	return _mediumFilter.getMediums( this );
}

std::list< common::CMedium< ClientResponses > *>
CErrorMediumProvider::getErrorMedium()
{
	std::list< common::CMedium< ClientResponses > *> mediums;
	mediums.push_back( CErrorMedium::getInstance() );
	return mediums;
}


}
