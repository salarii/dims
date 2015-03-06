// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef	MEDIUM_H
#define MEDIUM_H

#include "commonResponses.h"

#include "visitorConfigurationUtilities.h"

#include "tracker/trackerRequestsList.h"
#include "tracker/configureTrackerActionHandler.h"
#include "client/nodeRequestList.h"
#include "client/configureClientActionHadler.h"
#include "monitor/monitorRequestsList.h"
#include "monitor/configureMonitorActionHandler.h"
#include "seed/configureSeedActionHandler.h"
#include "seed/seedRequestsList.h"

namespace common
{
//fix  stupid look  of  all those  mediums
template < class _RequestResponses >
struct CRequest;

template < class _RequestResponses >
class CMedium
{
public:
	typedef _RequestResponses ResponsesType;
public:
	virtual bool serviced() const = 0;
	virtual bool flush() = 0;
	virtual void prepareMedium(){};
	virtual bool getResponseAndClear( std::multimap< CRequest< ResponsesType >const*, ResponsesType > & _requestResponse) = 0;// the order of  elements with the same key is important, I have read somewhere that in this c++ standard this is not guaranteed but "true in practice":  is  such assertion good  enough??
	virtual ~CMedium(){};
};

class CTrackerBaseMedium : public CMedium< tracker::TrackerResponses >
{
public:
	using CMedium::ResponsesType;
public:
	virtual void add( tracker::CGetBalanceRequest const * _request ){};
	virtual void add( tracker::CValidateTransactionsRequest const * _request ){};
	virtual void add( tracker::CConnectToTrackerRequest const * _request ){};
	virtual void add( common::CSendIdentifyDataRequest<tracker::TrackerResponses> const * _request ){};
	virtual void add( tracker::CAskForTransactionsRequest const * _request ){};
	virtual void add( tracker::CSetBloomFilterRequest const * _request ){};
	virtual void add( tracker::CGetSynchronizationInfoRequest const * _request ){};
	virtual void add( tracker::CGetNextBlockRequest const * _request ){};
	virtual void add( tracker::CSetNextBlockRequest< tracker::CDiskBlock > const * _request ){};
	virtual void add( tracker::CSetNextBlockRequest< tracker::CSegmentHeader > const * _request ){};
	virtual void add( tracker::CTransactionsStatusRequest const * _request ){};
	virtual void add( common::CNetworkRoleRequest< tracker::TrackerResponses > const * _request ){};
	virtual void add( common::CKnownNetworkInfoRequest< tracker::TrackerResponses > const * _request ){};
	virtual void add( common::CAckRequest< tracker::TrackerResponses > const * _request ){};
	virtual void add( common::CEndRequest< tracker::TrackerResponses > const * _request ){};
	virtual void add( common::CResultRequest< tracker::TrackerResponses > const * _request ){};
	virtual void add( tracker::CTransactionsPropagationRequest const * _request ){};
	virtual void add( tracker::CPassMessageRequest const * _request ){};
	virtual void add( tracker::CDeliverInfoRequest const * _request ){};
	virtual void add( common::CTimeEventRequest< tracker::TrackerResponses > const * _request ){};

	virtual ~CTrackerBaseMedium(){};
};

class CMonitorBaseMedium : public CMedium< monitor::MonitorResponses >
{
public:
	using CMedium::ResponsesType;
public:
	virtual void add( common::CSendIdentifyDataRequest< monitor::MonitorResponses > const * _request ){};
	virtual void add( common::CKnownNetworkInfoRequest< monitor::MonitorResponses > const * _request ){};
	virtual void add( common::CAckRequest< monitor::MonitorResponses > const * _request ){};
	virtual void add( common::CNetworkRoleRequest< monitor::MonitorResponses > const * _request ){};
	virtual void add( monitor::CConnectToNodeRequest const * _request ){};
	virtual void add( monitor::CConnectCondition const * _request ){};
	virtual void add( monitor::CInfoRequest const * _request ){};
	virtual void add( common::CTimeEventRequest< monitor::MonitorResponses > const * _request ){};
};

class CClientBaseMedium : public CMedium< client::ClientResponses >
{
public:
	using CMedium::ResponsesType;
public:
	virtual void add(client::CBalanceRequest const * _request ){};
	virtual void add( client::CTransactionStatusRequest const * _request ){};
	virtual void add( client::CTransactionSendRequest const * _request ){};
	virtual void add(client:: CTrackersInfoRequest const * _request ){};
	virtual void add( client::CMonitorInfoRequest const * _request ){};
	virtual void add( client::CDnsInfoRequest const * _request ){};
	virtual void add( client::CRecognizeNetworkRequest const * _request ){};
	virtual void add( client::CErrorForAppPaymentProcessing const * _request ){};
	virtual void add( client::CProofTransactionAndStatusRequest const * _request ){};
	virtual void add( common::CTimeEventRequest< client::ClientResponses > const * _request ){};
};

class CSeedBaseMedium : public CMedium< seed::SeedResponses >
{
public:
	using CMedium::ResponsesType;
public:
	virtual void add( common::CSendIdentifyDataRequest< seed::SeedResponses > const * _request ){};
	virtual void add( common::CConnectToNodeRequest< seed::SeedResponses > const * _request ){};
	virtual void add( common::CNetworkRoleRequest< seed::SeedResponses > const * _request ){};
	virtual void add( common::CAckRequest< seed::SeedResponses > const * _request ){};
	virtual void add( common::CKnownNetworkInfoRequest< seed::SeedResponses > const * _request ){};
	virtual void add( common::CTimeEventRequest< seed::SeedResponses > const * _request ){};
};

}
#endif // MEDIUM_H
