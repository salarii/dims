// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef	MEDIUM_H
#define MEDIUM_H

#include "commonResponses.h"

#include "visitorConfigurationUtilities.h"

#include "common/types.h"

namespace common
{

template < class _Types >
class CSendIdentifyDataRequest;

template < class _Types >
class CNetworkRoleRequest;

template < class _Types >
class CKnownNetworkInfoRequest;

template < class _Types >
class CAckRequest;

template < class _Types >
class CEndRequest;

template < class _Types >
class CResultRequest;

template < class _Types >
class CTimeEventRequest;

template < class _Types >
class CPingRequest;

template < class _Types >
class CPongRequest;

template < class _Types >
class CScheduleActionRequest;

template < class _Types >
class CInfoAskRequest;

template < class _Types >
class CConnectToNodeRequest;

template < class _Block, class _Types >
class CSetNextBlockRequest;

template < class _Types >
class CAskForTransactionsRequest;
}

namespace tracker
{
class CValidateTransactionsRequest;
class CConnectToTrackerRequest;
class CAskForTransactionsRequest;
class CSetBloomFilterRequest;
class CGetSynchronizationInfoRequest;
class CGetNextBlockRequest;
class CTransactionsStatusRequest;
class CTransactionsPropagationRequest;
class CPassMessageRequest;
class CDeliverInfoRequest;
class CGetBalanceRequest;
class CAskForRegistrationRequest;
class CRegisterProofRequest;

}

namespace client
{
struct CBalanceRequest;
struct CTransactionStatusRequest;
struct CTransactionSendRequest;
struct CTrackersInfoRequest;
struct CMonitorInfoRequest;
struct CDnsInfoRequest;
struct CRecognizeNetworkRequest;
struct CErrorForAppPaymentProcessing;
struct CProofTransactionAndStatusRequest;
}

namespace monitor
{
class CConnectToNodeRequest;
class CRegistrationTerms;
class CInfoRequest;
}

namespace common
{

template < class _Type >
class CAction;

//fix  stupid look  of  all those  mediums
template < class _RequestResponses >
class CRequest;

template < class _Type >
class CMedium
{
public:
	typedef _Type types;
public:
	virtual bool serviced() const = 0;
	virtual bool flush() = 0;
	virtual void prepareMedium(){};
	virtual void deleteRequest( CRequest< _Type >const* _request ){};// needed in some cases
	virtual bool getResponseAndClear( std::multimap< CRequest< _Type >const*, typename _Type::Response > & _requestResponse) = 0;// the order of  elements with the same key is important, I have read somewhere that in this c++ standard this is not guaranteed but "true in practice":  is  such assertion good  enough??
	virtual bool getDirectActionResponseAndClear( CAction< _Type >const * _action, std::list< typename _Type::Response > & _responses ){ return false; }
	virtual void deleteAction( CAction< _Type >const * _action ){};
	void registerDeleteHook( boost::signals2::slot< void () > const & _deleteHook )
	{
		m_deleteHook.connect( _deleteHook );
	}

	virtual ~CMedium()
	{
		m_deleteHook();
	};

	boost::signals2::signal<void ()> m_deleteHook;
};

class CTrackerBaseMedium : public CMedium< CTrackerTypes >
{
public:
	using CMedium::types;
public:
	virtual void add( common::CAckRequest< CTrackerTypes > const * _request ){};
	virtual void add( common::CPingRequest< CTrackerTypes > const * _request ){};
	virtual void add( common::CPongRequest< CTrackerTypes > const * _request ){};
	virtual void add( common::CEndRequest< CTrackerTypes > const * _request ){};
	virtual void add( common::CResultRequest< CTrackerTypes > const * _request ){};
	virtual void add( common::CSendIdentifyDataRequest<CTrackerTypes> const * _request ){};
	virtual void add( common::CNetworkRoleRequest< CTrackerTypes > const * _request ){};
	virtual void add( common::CKnownNetworkInfoRequest< CTrackerTypes > const * _request ){};
	virtual void add( common::CTimeEventRequest< CTrackerTypes > const * _request ){};
	virtual void add( common::CScheduleActionRequest< CTrackerTypes > const * _request ){};
	virtual void add( common::CInfoAskRequest< CTrackerTypes > const * _request ){};
	virtual void add( common::CSetNextBlockRequest< common::CDiskBlock, CTrackerTypes > const * _request ){};
	virtual void add( common::CSetNextBlockRequest< common::CSegmentHeader, CTrackerTypes > const * _request ){};
	virtual void add( tracker::CGetBalanceRequest const * _request ){};
	virtual void add( tracker::CValidateTransactionsRequest const * _request ){};
	virtual void add( tracker::CConnectToTrackerRequest const * _request ){};
	virtual void add( tracker::CAskForTransactionsRequest const * _request ){};
	virtual void add( tracker::CSetBloomFilterRequest const * _request ){};
	virtual void add( tracker::CGetSynchronizationInfoRequest const * _request ){};
	virtual void add( tracker::CGetNextBlockRequest const * _request ){};
	virtual void add( tracker::CTransactionsStatusRequest const * _request ){};
	virtual void add( tracker::CTransactionsPropagationRequest const * _request ){};
	virtual void add( tracker::CPassMessageRequest const * _request ){};
	virtual void add( tracker::CDeliverInfoRequest const * _request ){};
	virtual void add( tracker::CAskForRegistrationRequest const * _request ){};
	virtual void add( tracker::CRegisterProofRequest const * _request ){};

	virtual ~CTrackerBaseMedium(){};
};

class CMonitorBaseMedium : public CMedium< CMonitorTypes >
{
public:
	using CMedium::types;
public:
	virtual void add( common::CSendIdentifyDataRequest< CMonitorTypes > const * _request ){};
	virtual void add( common::CPingRequest< CMonitorTypes > const * _request ){};
	virtual void add( common::CPongRequest< CMonitorTypes > const * _request ){};
	virtual void add( common::CKnownNetworkInfoRequest< CMonitorTypes > const * _request ){};
	virtual void add( common::CAckRequest< CMonitorTypes > const * _request ){};
	virtual void add( common::CNetworkRoleRequest< CMonitorTypes > const * _request ){};
	virtual void add( common::CTimeEventRequest< CMonitorTypes > const * _request ){};
	virtual void add( common::CScheduleActionRequest< CMonitorTypes > const * _request ){};
	virtual void add( common::CInfoAskRequest< CMonitorTypes > const * _request ){};
	virtual void add( common::CResultRequest< CMonitorTypes > const * _request ){};
	virtual void add( common::CAskForTransactionsRequest< CMonitorTypes > const * _request ){};
	virtual void add( monitor::CRegistrationTerms const * _request ){};
	virtual void add( monitor::CInfoRequest const * _request ){};
	virtual void add( monitor::CConnectToNodeRequest const * _request ){};
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
	virtual void add( common::CPingRequest< CSeedTypes > const * _request ){};
	virtual void add( common::CPongRequest< CSeedTypes > const * _request ){};
	virtual void add( common::CConnectToNodeRequest< CSeedTypes > const * _request ){};
	virtual void add( common::CNetworkRoleRequest< CSeedTypes > const * _request ){};
	virtual void add( common::CAckRequest< CSeedTypes > const * _request ){};
	virtual void add( common::CKnownNetworkInfoRequest< CSeedTypes > const * _request ){};
	virtual void add( common::CInfoAskRequest< CSeedTypes > const * _request ){};
	virtual void add( common::CTimeEventRequest< CSeedTypes > const * _request ){};
};

template < typename _Class >
struct CGetType
{
	typedef int type;
};

template <>
struct CGetType< CTrackerBaseMedium >
{
	typedef CTrackerBaseMedium::types type;
};

template <>
struct CGetType< CMonitorBaseMedium >
{
	typedef CMonitorBaseMedium::types type;
};

template <>
struct CGetType< CClientBaseMedium >
{
	typedef CClientBaseMedium::types type;
};

template <>
struct CGetType< CSeedBaseMedium >
{
	typedef CSeedBaseMedium::types type;
};

}
#endif // MEDIUM_H
