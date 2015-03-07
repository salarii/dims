// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "monitor/noMediumHandling.h"

namespace monitor
{

class CErrorMedium : public common::CMonitorBaseMedium
{
public:
	virtual bool serviced() const;

	virtual bool flush(){ return true; }

	virtual bool getResponseAndClear( std::multimap< common::CRequest< common::CMonitorTypes >const*, MonitorResponses > & _requestResponse );

	virtual void add( CInfoRequest const * _request );

	static CErrorMedium* getInstance();
		CErrorMedium();
private:
	void clearResponses();
private:

	std::multimap< common::CRequest< common::CMonitorTypes >const*, MonitorResponses > m_responses;

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

void
CErrorMedium::add( CInfoRequest const *_request )
{
	m_responses.insert( std::make_pair( (common::CRequest< common::CMonitorTypes >*)_request, common::CNoMedium() ) );
}

bool
CErrorMedium::serviced() const
{
	return true;
}


bool
CErrorMedium::getResponseAndClear( std::multimap< common::CRequest< common::CMonitorTypes >const*,  MonitorResponses > & _requestResponse )
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

std::list< common::CMonitorBaseMedium *>
CErrorMediumProvider::provideConnection( common::CMonitorMediumFilter const & _filter )
{
	return m_mediums;
}

}
