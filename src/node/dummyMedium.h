// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef	DUMMY_MEDIUM_H
#define DUMMY_MEDIUM_H

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


class CDummyMedium : public common::CMedium< NodeResponses >
{
public:
	CDummyMedium();

	bool serviced() const;

	void add( CRequest const * _request );

	void add( CTrackersInfoRequest const * _request );

	bool flush();

	bool getResponse( std::vector< node::NodeResponses > & _requestResponse ) const;

	void clearResponses();
private:
	std::list< CTrackersInfoRequest const * > m_trackerStatsRequests;
	bool m_serviced;

	common::CTrackerStats const m_trackerStats;
	std::vector< node::NodeResponses > m_requestResponse;
};


}

#endif // DUMMY_MEDIUM_H
