// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/foreach.hpp>
#include <exception> 

#include "dummyMedium.h"
#include "sendInfoRequestAction.h"

#include "support.h"
#include "tracker/nodeMessages.h"

namespace node
{

CDummyMedium::CDummyMedium()
	: m_serviced( true )
	, m_trackerInfo( "127.0.0.1:10", "0.005", "100" )
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
	try
	{
		CTrackersInfoRequest const * trackerInfoRequest;

		trackerInfoRequest = dynamic_cast< CTrackersInfoRequest const  *>(_request);

		m_trackerInfoRequests.push_back( trackerInfoRequest );

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
	CBufferAsStream stream( (char*)m_buffer.m_buffer, MaxBufferSize, SER_DISK, CLIENT_VERSION);

	BOOST_FOREACH( CTrackersInfoRequest const * request, m_trackerInfoRequests )
	{
		serializeEnum( stream, self::CServerMessageType::TrackerInfo );

		writeTrackerInfo( stream , m_trackerInfo, request->m_reqInfo );
	}
	m_serviced = true;

	m_buffer.m_usedSize = stream.getMaxWritePosition();

	m_trackerInfoRequests.clear();
}

bool
CDummyMedium::getResponse( CCommunicationBuffer & _outBuffor ) const
{
	_outBuffor = m_buffer;
}

}