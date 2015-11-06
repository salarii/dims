// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/noMediumHandling.h"

namespace common
{

class CErrorMedium : public common::CMedium
{
public:
	bool serviced() const;

	bool flush(){ return true; }

	void deleteRequest( CRequest const* _request )
	{
		m_responses.erase( _request );
	}

	bool getResponseAndClear( std::multimap< common::CRequest const*, common::DimsResponse > & _requestResponse );

	bool getDirectActionResponseAndClear( CAction const * _action, std::list< DimsResponse > & _responses )
	{
		_responses.push_back( CNoMedium() );
		return true;
	}

	void add( common::CSendMessageRequest const * _request )
	{
		m_responses.insert( make_pair( (CRequest*)_request, CNoMedium() ) );
	}

	static CErrorMedium* getInstance();

	CErrorMedium();
private:
	void clearResponses();
private:

	std::multimap< common::CRequest const*, common::DimsResponse > m_responses;

	static CErrorMedium * ms_instance;
};



CErrorMedium * CErrorMedium::ms_instance = NULL;

CErrorMedium*
CErrorMedium::getInstance()
{
	if ( !ms_instance )
	{
		ms_instance = new CErrorMedium();
	};
	return ms_instance;
}

CErrorMedium::CErrorMedium()
{
}

bool
CErrorMedium::serviced() const
{
	return true;
}


bool
CErrorMedium::getResponseAndClear( std::multimap< common::CRequest const*,  common::DimsResponse > & _requestResponse )
{
	_requestResponse = m_responses;
	clearResponses();
	return true;
}

void
CErrorMedium::clearResponses()
{
	m_responses.clear();
}

CErrorMediumProvider * CErrorMediumProvider::ms_instance = NULL;

CErrorMediumProvider*
CErrorMediumProvider::getInstance( )
{
	if ( !ms_instance )
	{
		ms_instance = new CErrorMediumProvider();
	};
	return ms_instance;
}

CErrorMediumProvider::CErrorMediumProvider()
{
	m_mediums.push_back( CErrorMedium::getInstance() );
}

std::list< common::CMedium *>
CErrorMediumProvider::provideConnection( common::CMediumFilter const & _filter )
{
	return m_mediums;
}

}
