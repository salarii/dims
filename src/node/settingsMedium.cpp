// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/foreach.hpp>
#include <exception> 

#include "settingsMedium.h"
#include "sendInfoRequestAction.h"

#include "helper.h"
#include "common/nodeMessages.h"

#include "common/ratcoinParams.h"

using namespace common;

namespace node
{

CSettingsMedium::CSettingsMedium()
	: m_serviced( true )
	, m_trackerStats( "local", 0, 0,"127.0.0.1", common::ratcoinParams().getDefaultClientPort() )
{
}

void
CSettingsMedium::setTrackerIp( std::string const & _ip )
{
	m_trackerStats.m_ip = _ip;
}

bool
CSettingsMedium::serviced() const
{
	return m_serviced;
}

void
CSettingsMedium::add( CRequest const * _request )
{
}

void
CSettingsMedium::add( CTrackersInfoRequest const * _request )
{
	try
	{
		m_trackerStatsRequests.push_back( _request );

		m_serviced = false;

	}
	catch (std::exception& _ex)
	{
		// service it at some point
	}
}

bool
CSettingsMedium::flush()
{

	BOOST_FOREACH( CTrackersInfoRequest const * request, m_trackerStatsRequests )
	{
		m_requestResponse.push_back( m_trackerStats );
	}
	m_serviced = true;

	m_trackerStatsRequests.clear();
}

bool
CSettingsMedium::getResponse( std::vector< node::NodeResponses > & _requestResponse ) const
{
	_requestResponse = m_requestResponse;
}

void
CSettingsMedium::clearResponses()
{
	m_requestResponse.clear();
}

}
