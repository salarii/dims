// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef	MEDIUM_H
#define MEDIUM_H

#include "requestResponse.h"

#include "visitorConfigurationUtilities.h"

#include "tracker/trackerRequestsList.h"
#include "tracker/configureTrackerActionHandler.h"
#include "client/nodeRequestList.h"
#include "client/configureNodeActionHadler.h"
#include "monitor/monitorRequestsList.h"
#include "monitor/configureMonitorActionHandler.h"
#include "seed/configureSeedActionHandler.h"
#include "seed/seedRequestsList.h"

namespace common
{

template < class _RequestResponses >
struct CRequest;

template < class _RequestResponses >
class CMedium
{
public:
	virtual bool serviced() const = 0;
	virtual bool flush() = 0;
	virtual void clearResponses() = 0;
	virtual bool getResponse( std::vector< _RequestResponses > & _requestResponse ) const = 0;
	virtual void add( CRequest< _RequestResponses > const * _request ) = 0;
	virtual ~CMedium(){};
};

template <>
class CMedium< tracker::TrackerResponses >
{
public:
	virtual bool serviced() const = 0;
	virtual bool flush() = 0;

	virtual bool getResponse( std::vector< tracker::TrackerResponses > & _requestResponse ) const = 0;
	virtual void clearResponses() = 0;

	virtual void add( tracker::CGetBalanceRequest const * _request ){};
	virtual void add( tracker::CValidateTransactionsRequest const * _request ){};
	virtual void add( tracker::CConnectToTrackerRequest const * _request ){};
	virtual void add( common::CIdentifyRequest<tracker::TrackerResponses> const * _request ){};
	virtual void add( common::CIdentifyResponse<tracker::TrackerResponses> const * _request ){};
	virtual void add( common::CContinueReqest<tracker::TrackerResponses> const * _request ){};
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
	virtual ~CMedium(){};
};

template <>
class CMedium< client::NodeResponses >
{
public:
	virtual bool serviced() const = 0;
	virtual bool flush() = 0;

	virtual bool getResponse( std::vector< client::NodeResponses > & _requestResponse ) const = 0;
	virtual void clearResponses() = 0;
	virtual void add(client::CBalanceRequest const * _request ){};
	virtual void add(client:: CInfoRequestContinueComplex const * _request ){};
	virtual void add( client::CInfoRequestContinue const * _request ){};
	virtual void add( client::CTransactionStatusRequest const * _request ){};
	virtual void add( client::CTransactionSendRequest const * _request ){};
	virtual void add(client:: CTrackersInfoRequest const * _request ){};
	virtual void add( client::CMonitorInfoRequest const * _request ){};
	virtual void add( client::CDnsInfoRequest const * _request ){};
	virtual void add( common::CContinueReqest< client::NodeResponses > const * _request ){};
	virtual void add( client::CRecognizeNetworkRequest const * _request ){};
	virtual void add( client::CErrorForAppPaymentProcessing const * _request ){};
	virtual void add( client::CProofTransactionAndStatusRequest const * _request ){};
	virtual ~CMedium(){};
};

template <>
class CMedium< monitor::MonitorResponses >
{
public:
	virtual bool serviced() const = 0;
	virtual bool flush() = 0;

	virtual bool getResponse( std::vector< monitor::MonitorResponses > & _requestResponse ) const = 0;
	virtual void clearResponses() = 0;
	virtual void add( common::CIdentifyRequest< monitor::MonitorResponses > const * _request ){};
	virtual void add( common::CContinueReqest< monitor::MonitorResponses > const * _request ){};
	virtual void add( common::CIdentifyResponse< monitor::MonitorResponses > const * _request ){};
	virtual void add( common::CKnownNetworkInfoRequest< monitor::MonitorResponses > const * _request ){};
	virtual void add( common::CAckRequest< monitor::MonitorResponses > const * _request ){};
	virtual void add( common::CNetworkRoleRequest< monitor::MonitorResponses > const * _request ){};
	virtual void add( monitor::CConnectToNodeRequest const * _request ){};
	virtual void add( monitor::CConnectCondition const * _request ){};
	virtual void add( monitor::CInfoRequest const * _request ){};
	virtual ~CMedium(){};
};

template <>
class CMedium< seed::SeedResponses >
{
public:
	virtual bool serviced() const = 0;
	virtual bool flush() = 0;

	virtual bool getResponse( std::vector< seed::SeedResponses > & _requestResponse ) const = 0;
	virtual void clearResponses() = 0;
	virtual void add( common::CIdentifyRequest< seed::SeedResponses > const * _request ){};
	virtual void add( common::CContinueReqest< seed::SeedResponses > const * _request ){};
	virtual void add( common::CIdentifyResponse< seed::SeedResponses > const * _request ){};
	virtual void add( common::CConnectToNodeRequest< seed::SeedResponses > const * _request ){};
	virtual void add( common::CNetworkRoleRequest< seed::SeedResponses > const * _request ){};
	virtual void add( common::CAckRequest< seed::SeedResponses > const * _request ){};
	virtual void add( common::CKnownNetworkInfoRequest< seed::SeedResponses > const * _request ){};
	virtual ~CMedium(){};
};

}
#endif // MEDIUM_H
