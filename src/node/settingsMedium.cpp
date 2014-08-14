// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/foreach.hpp>
#include <exception> 

#include "settingsMedium.h"
#include "sendInfoRequestAction.h"

#include "helper.h"
#include "common/nodeMessages.h"
#include "common/commonEvents.h"
#include "common/ratcoinParams.h"
#include "common/commonResponses.h"
#include "clientResponses.h"

using namespace common;

namespace client
{

CSettingsMedium::CSettingsMedium()
	: m_serviced( true )
	, m_trackerStats( CPubKey(), 0, 0,"127.0.0.1" )
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
		m_requestResponse.push_back( m_trackerStats );

		m_serviced = false;

	}
	catch (std::exception& _ex)
	{
		// service it at some point
	}
}

void
CSettingsMedium::add( CDnsInfoRequest const * _request )
{
	vector< CAddress > addresses;
	getSeedIps( addresses );

	CDnsInfo dnsInfo( addresses );

	m_requestResponse.push_back( dnsInfo );
}

void
CSettingsMedium::getSeedIps( vector<CAddress> & _vAdd )
{
	const vector<CDNSSeedData> &vSeeds = ratcoinParams().DNSSeeds();

	BOOST_FOREACH(const CDNSSeedData &seed, vSeeds) {
		if (HaveNameProxy()) {
			AddOneShot(seed.host);
		} else {
			vector<CNetAddr> vIPs;

			if (LookupHost(seed.host.c_str(), vIPs))
			{
				BOOST_FOREACH(CNetAddr& ip, vIPs)
				{
					int nOneDay = 24*3600;
					CAddress addr = CAddress(CService(ip, ratcoinParams().GetDefaultPort()));
					addr.nTime = GetTime() - 3*nOneDay - GetRand(4*nOneDay); // use a random age between 3 and 7 days old
					_vAdd.push_back(addr);
				}
			}

		}
	}
}

void
CSettingsMedium::add( common::CContinueReqest< NodeResponses > const * _request )
{
	m_requestResponse.push_back( common::CContinueResult( _request->getRequestId() ) );
}


bool
CSettingsMedium::flush()
{
	m_serviced = true;
}

bool
CSettingsMedium::getResponse( std::vector< client::NodeResponses > & _requestResponse ) const
{
	_requestResponse = m_requestResponse;
}

void
CSettingsMedium::clearResponses()
{
	m_requestResponse.clear();
}

}
