// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "errorMediumProvider.h"

#include "clientFilters.h"

namespace client
{

class CErrorMedium : public common::CMedium
{
public:

	static CErrorMedium * getInstance();

	bool serviced() const;

	void add( common::CRequest const * _request ){};

	void add( common::CSendMessageRequest const * _request )
	{
		m_requestResponse.insert( std::make_pair( ( common::CRequest * )_request, common::CNoMedium() ) );
	}

	bool flush(){ return true; };

	bool getResponseAndClear( std::multimap< common::CRequest const*, common::DimsResponse > & _requestResponse );
private:
	void clearResponses();

	CErrorMedium(){};

	void getSeedIps( vector<CAddress> & _vAdd );
private:
	static CErrorMedium * ms_instance;

	std::multimap< common::CRequest const*, common::DimsResponse > m_requestResponse;
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
CErrorMedium::getResponseAndClear( std::multimap< common::CRequest const*, common::DimsResponse > & _requestResponse )
{
	_requestResponse = m_requestResponse;

	clearResponses();

	return true;
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

std::list< common::CMedium *>
CErrorMediumProvider::provideConnection( common::CMediumFilter const & _mediumFilter )
{
	return _mediumFilter.getMediums( this );
}

std::list< common::CMedium *>
CErrorMediumProvider::getErrorMedium()
{
	std::list< common::CMedium *> mediums;
	mediums.push_back( CErrorMedium::getInstance() );
	return mediums;
}


}

