// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef	SETTINGS_MEDIUM_H
#define SETTINGS_MEDIUM_H

#include <list>
#include <string>

#include "common/medium.h"
#include "common/commonResponses.h"
#include "common/communicationBuffer.h"
#include "configureClientActionHadler.h"

namespace common
{

template < class ResponsesType >
class CContinueReqest;
}


namespace client
{

class CTrackersInfoRequest;
struct CMonitorInfoRequest;

class CDefaultMedium : public common::CMedium< ClientResponses >
{
public:

	static CDefaultMedium * getInstance();

	bool serviced() const;

	void add( common::CRequest< ClientResponses > const * _request );

	void add( CDnsInfoRequest const * _request );

	void add( CMonitorInfoRequest const * _request );

	void add( CTrackersInfoRequest const * _request );

	void add( CBalanceRequest const * _request );

	bool flush();

	bool getResponseAndClear( std::multimap< common::CRequest< ClientResponses >const*, ClientResponses > & _requestResponse );
private:
	void clearResponses();

	CDefaultMedium();

	void getSeedIps( vector<CAddress> & _vAdd );
private:
	static CDefaultMedium * ms_instance;

	bool m_serviced;
	std::multimap< common::CRequest< ClientResponses >const*, ClientResponses > m_requestResponse;
};


}

#endif // SETTINGS_MEDIUM_H
