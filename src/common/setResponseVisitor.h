// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SET_RESPONSE_VISITOR_H
#define SET_RESPONSE_VISITOR_H

#include <boost/variant.hpp> 
#include "common/types.h"

namespace tracker
{

class CGetBalanceAction;

class CValidateTransactionsAction;

class CConnectNodeAction;

class CTrackOriginAddressAction;

class CSynchronizationAction;

class CProvideInfoAction;

class CPingAction;

class CRegisterAction;

class CRecognizeNetworkAction;
}

namespace client
{

class CSendBalanceInfoAction;

class CSendTransactionAction;

class CConnectAction;

class CPayLocalApplicationAction;

class CSendInfoRequestAction;
}

namespace monitor
{
class CConnectNodeAction;

class CUpdateDataAction;

class CAdmitTrackerAction;

class CAdmitProofTransactionBundle;

class CPingAction;

class CRecognizeNetworkAction;

class CTrackOriginAddressAction;
}

namespace seed
{
class CAcceptNodeAction;

class CPingAction;
}

namespace common
{

template < class _RequestResponses > class CAction;

template < class _Type >
class CSetResponseVisitor
{
public:
	CSetResponseVisitor( _Type const & _requestRespond );
	virtual void visit( CAction< _Type > & _action );
};

// weak spot because one have to remembar to add function for  every new action refactor it??
template<>
class CSetResponseVisitor< CTrackerTypes >
{
public:
	CSetResponseVisitor( tracker::TrackerResponses const & _trackerResponses );

	virtual void visit( common::CAction< CTrackerTypes > & _action );

	virtual void visit( tracker::CGetBalanceAction & _action );

	virtual void visit( tracker::CValidateTransactionsAction & _action );

	virtual void visit( tracker::CConnectNodeAction & _action );

	virtual void visit( tracker::CTrackOriginAddressAction & _action );

	virtual void visit( tracker::CSynchronizationAction & _action );

	virtual void visit( tracker::CProvideInfoAction & _action );

	virtual void visit( tracker::CPingAction & _action );

	virtual void visit( tracker::CRegisterAction & _action );

	virtual void visit( tracker::CRecognizeNetworkAction & _action );
private:
	tracker::TrackerResponses m_trackerResponses;
};

template<>
class CSetResponseVisitor< CClientTypes >
{
public:
	CSetResponseVisitor( client::ClientResponses const & _requestRespond );

	void visit( client::CSendTransactionAction & _action );

	void visit( client::CConnectAction & _action );

	void visit( CAction< client::ClientResponses > & _action );

	void visit( client::CSendBalanceInfoAction & _action );

	void visit( client::CPayLocalApplicationAction & _action );

	void visit( client::CSendInfoRequestAction & _action ){};
private:

	client::ClientResponses m_requestResponse;
};


template<>
class CSetResponseVisitor< CMonitorTypes>
{
public:
	CSetResponseVisitor( monitor::MonitorResponses const & _requestResponse );

	void visit( monitor::CConnectNodeAction & _action );

	virtual void visit( monitor::CUpdateDataAction & _action );

	virtual void visit( monitor::CAdmitTrackerAction & _action );

	virtual void visit( monitor::CAdmitProofTransactionBundle & _action );

	virtual void visit( monitor::CPingAction & _action );

	virtual void visit( monitor::CRecognizeNetworkAction & _action );

	virtual void visit( monitor::CTrackOriginAddressAction & _action );
private:

	monitor::MonitorResponses m_requestResponse;
};

template<>
class CSetResponseVisitor< CSeedTypes >
{
public:
	CSetResponseVisitor( seed::SeedResponses const & _requestResponse );

	void visit( seed::CAcceptNodeAction & _action );

	void visit( seed::CPingAction & _action );
private:
	seed::SeedResponses m_requestResponse;
};


}

#endif
