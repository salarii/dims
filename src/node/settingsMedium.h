// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef	SETTINGS_MEDIUM_H
#define SETTINGS_MEDIUM_H

#include <list>
#include <string>

#include "common/medium.h"
#include "common/requestResponse.h"
#include "common/communicationBuffer.h"
#include "configureNodeActionHadler.h"

namespace node
{

class CTrackersInfoRequest;
struct CRequest;


class CSettingsMedium : public common::CMedium< NodeResponses >
{
public:
	CSettingsMedium();

	bool serviced() const;

	void add( CRequest const * _request );

	void add( CTrackersInfoRequest const * _request );

	bool flush();

	bool getResponse( std::vector< node::NodeResponses > & _requestResponse ) const;

	void clearResponses();

	void setTrackerIp( std::string const & _ip );
private:
	std::list< CTrackersInfoRequest const * > m_trackerStatsRequests;
	bool m_serviced;

	common::CTrackerStats m_trackerStats;
	std::vector< node::NodeResponses > m_requestResponse;
};


}

#endif // SETTINGS_MEDIUM_H
