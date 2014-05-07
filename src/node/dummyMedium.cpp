// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/foreach.hpp>
#include <exception> 

#include "dummyMedium.h"
#include "sendInfoRequestAction.h"

#include "helper.h"
#include "common/nodeMessages.h"

#include "common/ratcoinParams.h"

using namespace common;

namespace node
{

CDummyMedium::CDummyMedium()
	: m_serviced( true )
    , m_trackerStats( "dummy", 0.005, 100,"127.0.0.1", common::ratcoinParams().getDefaultClientPort() )
{
}

bool
CDummyMedium::serviced() const
{
	return m_serviced;
}

void
CDummyMedium::add( CRequest const * _request )
{
}

void
CDummyMedium::add( CTrackersInfoRequest const * _request )
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
CDummyMedium::flush()
{

	BOOST_FOREACH( CTrackersInfoRequest const * request, m_trackerStatsRequests )
	{
		m_requestResponse.push_back( m_trackerStats );
	}
	m_serviced = true;

	m_trackerStatsRequests.clear();
}

bool
CDummyMedium::getResponse( std::vector< node::NodeResponses > & _requestResponse ) const
{
	_requestResponse = m_requestResponse;
}

void
CDummyMedium::clearResponses()
{
	m_requestResponse.clear();
}

}
