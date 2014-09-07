// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef	MEDIUM_H
#define MEDIUM_H

#include "requestResponse.h"

#include "visitorConfigurationUtilities.h"

#include "tracker/trackerRequestsList.h"
#include "node/nodeRequestList.h"
#include "node/configureNodeActionHadler.h"
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
	virtual void add( tracker::CTransactionsKnownRequest const * _request ){};
	virtual void add( tracker::CTransactionsAckRequest const * _request ){};
	virtual void add( tracker::CTransactionsDoublespendRequest const * _request ){};
	virtual void add( tracker::CTransactionsNotOkRequest const * _request ){};
	virtual void add( common::CNetworkRoleRequest< tracker::TrackerResponses > const * _request ){};
	virtual void add( common::CKnownNetworkInfoRequest< tracker::TrackerResponses > const * _request ){};
	virtual void add( common::CAckRequest< tracker::TrackerResponses > const * _request ){};
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
	virtual void add( VisitorParam( client::NodeRequestsList ,0 ) const * _request ){};
	virtual void add( VisitorParam( client::NodeRequestsList ,1 ) const * _request ){};
	virtual void add( VisitorParam( client::NodeRequestsList ,2 ) const * _request ){};
	virtual void add( VisitorParam( client::NodeRequestsList ,3 ) const * _request ){};
	virtual void add( VisitorParam( client::NodeRequestsList ,4 ) const * _request ){};
	virtual void add( VisitorParam( client::NodeRequestsList ,5 ) const * _request ){};
	virtual void add( VisitorParam( client::NodeRequestsList ,6 ) const * _request ){};
	virtual void add( VisitorParam( client::NodeRequestsList ,7 ) const * _request ){};
	virtual void add( VisitorParam( client::NodeRequestsList ,8 ) const * _request ){};
	virtual void add( VisitorParam( client::NodeRequestsList ,9 ) const * _request ){};
	virtual void add( VisitorParam( client::NodeRequestsList ,10 ) const * _request ){};
	virtual void add( VisitorParam( client::NodeRequestsList ,11 ) const * _request ){};
	virtual void add( VisitorParam( client::NodeRequestsList ,12 ) const * _request ){};
	virtual void add( VisitorParam( client::NodeRequestsList ,13 ) const * _request ){};
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
	virtual void add( VisitorParam( monitor::MonitorRequestsList ,0 ) const * _request ){};
	virtual void add( VisitorParam( monitor::MonitorRequestsList ,1 ) const * _request ){};
	virtual void add( VisitorParam( monitor::MonitorRequestsList ,2 ) const * _request ){};
	virtual void add( VisitorParam( monitor::MonitorRequestsList ,3 ) const * _request ){};
	virtual void add( VisitorParam( monitor::MonitorRequestsList ,4 ) const * _request ){};
	virtual void add( VisitorParam( monitor::MonitorRequestsList ,5 ) const * _request ){};
	virtual void add( VisitorParam( monitor::MonitorRequestsList ,6 ) const * _request ){};
	virtual void add( VisitorParam( monitor::MonitorRequestsList ,7 ) const * _request ){};
	virtual void add( VisitorParam( monitor::MonitorResponseList ,8 ) const * _request ){};
	virtual void add( VisitorParam( monitor::MonitorRequestsList ,9 ) const * _request ){};
	virtual void add( VisitorParam( monitor::MonitorRequestsList ,10 ) const * _request ){};
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
	virtual void add( VisitorParam( seed::SeedRequestsList ,0 ) const * _request ){};
	virtual void add( VisitorParam( seed::SeedRequestsList ,1 ) const * _request ){};
	virtual void add( VisitorParam( seed::SeedRequestsList ,2 ) const * _request ){};
	virtual void add( VisitorParam( seed::SeedRequestsList ,3 ) const * _request ){};
	virtual void add( VisitorParam( seed::SeedRequestsList ,4 ) const * _request ){};
	virtual void add( VisitorParam( seed::SeedRequestsList ,5 ) const * _request ){};
	virtual void add( VisitorParam( seed::SeedRequestsList ,6 ) const * _request ){};
	virtual void add( VisitorParam( seed::SeedRequestsList ,7 ) const * _request ){};
	virtual void add( VisitorParam( seed::SeedRequestsList ,8 ) const * _request ){};
	virtual void add( VisitorParam( seed::SeedRequestsList ,9 ) const * _request ){};
	virtual void add( VisitorParam( seed::SeedRequestsList ,10 ) const * _request ){};
	virtual ~CMedium(){};
};

}

#endif // MEDIUM_H
