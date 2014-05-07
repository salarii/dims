// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef	DUMMY_MEDIUM_H
#define DUMMY_MEDIUM_H

#include <list>
#include <string>

#include "medium.h"
#include "requestRespond.h"
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
	bool flush();
	virtual bool getResponse( std::vector< NodeResponses > & _requestResponse ) const;
private:
	std::list< CTrackersInfoRequest const * > m_trackerStatsRequests;
	bool m_serviced;

	common::CCommunicationBuffer m_buffer;

	CTrackerStats const m_trackerStats;
};


}

#endif // DUMMY_MEDIUM_H
