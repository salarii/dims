// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/foreach.hpp>
#include <exception> 

#include "dummyMedium.h"
#include "sendInfoRequestAction.h"

namespace node
{

CDummyMedium::CDummyMedium()
	: m_serviced( false )
	, m_trackerInfo( "127.0.0.1:10", "0.005", "100" )
{
}

bool
CDummyMedium::serviced() const
{

}

void
CDummyMedium::add( CRequest const * _request )
{
	try
	{
		CTrackersInfoRequest const * trackerInfoRequest;

		trackerInfoRequest = dynamic_cast< CTrackersInfoRequest const  *>(_request);

		m_trackerInfoRequests.push_back( trackerInfoRequest );

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
		BOOST_FOREACH( TrackerInfo::Enum const info, request->m_reqInfo )
		{
			switch ( info )
			{
				case TrackerInfo::Ip:
					stream << m_trackerInfo.m_ip;
					break;
				case TrackerInfo::Price:
					stream << m_trackerInfo.m_price;
					break;
				case TrackerInfo::Rating:
					stream << m_trackerInfo.m_rating;
					break;
				default:
					;
			}
		}
	}

	m_trackerInfoRequests.clear();
}

bool
CDummyMedium::getResponse( CCommunicationBuffer & _outBuffor ) const
{
	_outBuffor = m_buffer;
}

}