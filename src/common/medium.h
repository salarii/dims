// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef	MEDIUM_H
#define MEDIUM_H

#include "commonResponses.h"

#include "visitorConfigurationUtilities.h"

#include "tracker/trackerRequestsList.h"
#include "client/nodeRequestList.h"
#include "monitor/monitorRequestsList.h"
#include "seed/seedRequestsList.h"

#include "common/types.h"

namespace common
{

//fix  stupid look  of  all those  mediums
template < class _RequestResponses >
struct CRequest;

template < class _Type >
class CMedium
{
public:
	typedef _Type types;
public:
	virtual bool serviced() const = 0;
	virtual bool flush() = 0;
	virtual void prepareMedium(){};
	virtual bool getResponseAndClear( std::multimap< CRequest< _Type >const*, RESPONSE_TYPE(_Type) > & _requestResponse) = 0;// the order of  elements with the same key is important, I have read somewhere that in this c++ standard this is not guaranteed but "true in practice":  is  such assertion good  enough??
	virtual ~CMedium(){};
};

class CTrackerBaseMedium : public CMedium< CTrackerTypes>
{
public:
	using CMedium::types;
public:
	virtual void add( common::CAckRequest< CTrackerTypes > const * _request ){};
	virtual void add( common::CEndRequest< CTrackerTypes > const * _request ){};
	virtual void add( common::CResultRequest< CTrackerTypes > const * _request ){};
	virtual void add( common::CSendIdentifyDataRequest<CTrackerTypes> const * _request ){};
	virtual void add( common::CNetworkRoleRequest< CTrackerTypes > const * _request ){};
	virtual void add( common::CKnownNetworkInfoRequest< CTrackerTypes > const * _request ){};
	virtual void add( common::CTimeEventRequest< CTrackerTypes > const * _request ){};
	virtual void add( tracker::CGetBalanceRequest const * _request ){};
	virtual void add( tracker::CValidateTransactionsRequest const * _request ){};
	virtual void add( tracker::CConnectToTrackerRequest const * _request ){};
	virtual void add( tracker::CAskForTransactionsRequest const * _request ){};
	virtual void add( tracker::CSetBloomFilterRequest const * _request ){};
	virtual void add( tracker::CGetSynchronizationInfoRequest const * _request ){};
	virtual void add( tracker::CGetNextBlockRequest const * _request ){};
	virtual void add( tracker::CSetNextBlockRequest< tracker::CDiskBlock > const * _request ){};
	virtual void add( tracker::CSetNextBlockRequest< tracker::CSegmentHeader > const * _request ){};
	virtual void add( tracker::CTransactionsStatusRequest const * _request ){};
	virtual void add( tracker::CTransactionsPropagationRequest const * _request ){};
	virtual void add( tracker::CPassMessageRequest const * _request ){};
	virtual void add( tracker::CDeliverInfoRequest const * _request ){};

	virtual ~CTrackerBaseMedium(){};
};

class CMonitorBaseMedium : public CMedium< CMonitorTypes >
{
public:
	using CMedium::types;
public:
	virtual void add( common::CSendIdentifyDataRequest< CMonitorTypes > const * _request ){};
	virtual void add( common::CKnownNetworkInfoRequest< CMonitorTypes > const * _request ){};
	virtual void add( common::CAckRequest< CMonitorTypes > const * _request ){};
	virtual void add( common::CNetworkRoleRequest< CMonitorTypes > const * _request ){};
	virtual void add( common::CTimeEventRequest< CMonitorTypes > const * _request ){};
	virtual void add( monitor::CConnectToNodeRequest const * _request ){};
	virtual void add( monitor::CConnectCondition const * _request ){};
	virtual void add( monitor::CInfoRequest const * _request ){};
};

class CClientBaseMedium : public CMedium< CClientTypes >
{
public:
	using CMedium::types;
public:
	virtual void add( common::CTimeEventRequest< CClientTypes > const * _request ){};
	virtual void add(client::CBalanceRequest const * _request ){};
	virtual void add( client::CTransactionStatusRequest const * _request ){};
	virtual void add( client::CTransactionSendRequest const * _request ){};
	virtual void add(client:: CTrackersInfoRequest const * _request ){};
	virtual void add( client::CMonitorInfoRequest const * _request ){};
	virtual void add( client::CDnsInfoRequest const * _request ){};
	virtual void add( client::CRecognizeNetworkRequest const * _request ){};
	virtual void add( client::CErrorForAppPaymentProcessing const * _request ){};
	virtual void add( client::CProofTransactionAndStatusRequest const * _request ){};
};

class CSeedBaseMedium : public CMedium< CSeedTypes >
{
public:
	using CMedium::types;
public:
	virtual void add( common::CSendIdentifyDataRequest< CSeedTypes > const * _request ){};
	virtual void add( common::CConnectToNodeRequest< CSeedTypes > const * _request ){};
	virtual void add( common::CNetworkRoleRequest< CSeedTypes > const * _request ){};
	virtual void add( common::CAckRequest< CSeedTypes > const * _request ){};
	virtual void add( common::CKnownNetworkInfoRequest< CSeedTypes > const * _request ){};
	virtual void add( common::CTimeEventRequest< CSeedTypes > const * _request ){};
};

/*
template < typename _Class >
struct CGetType
{
	typedef int type;
};
 Medium;

CMonitorBaseMedium Medium;

CClientBaseMedium Medium;

CSeedBaseMedium Medium;
template <>
struct CGetResponseType< CTrackerBaseMedium >
{
	typedef CTrackerBaseMedium::types type;
};

template <>
struct CGetResponseType< CMonitorTypes >
{
	typedef CMonitorTypes::Response type;
};

template <>
struct CGetResponseType< CClientTypes >
{
	typedef CClientTypes::Response type;
};
*/
}
#endif // MEDIUM_H
