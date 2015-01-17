// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SET_RESPONSE_VISITOR_H
#define SET_RESPONSE_VISITOR_H

#include <boost/variant.hpp> 
#include "tracker/configureTrackerActionHandler.h"
#include "node/configureNodeActionHadler.h"
#include "monitor/configureMonitorActionHandler.h"
#include "seed/configureSeedActionHandler.h"

namespace tracker
{

class CGetBalanceAction;

class CValidateTransactionsAction;

class CConnectNodeAction;

class CTrackOriginAddressAction;

class CSynchronizationAction;
}

namespace client
{

class CSendBalanceInfoAction;

class CSendTransactionAction;

class CConnectAction;

class CPayLocalApplicationAction;

}

namespace monitor
{
class CConnectNodeAction;
}

namespace seed
{
class CAcceptNodeAction;
}

namespace common
{

template < class _RequestResponses > class CAction;

template < class _RequestResponses >
class CSetResponseVisitor
{
public:
	CSetResponseVisitor( _RequestResponses const & _requestRespond );
	virtual void visit( CAction< _RequestResponses > & _action );
};

// weak spot because one have to remembar to ad function for  every new action refactor it??
template<>
class CSetResponseVisitor< tracker::TrackerResponses >
{
public:
	CSetResponseVisitor( tracker::TrackerResponses const & _trackerResponses );

	virtual void visit( common::CAction< tracker::TrackerResponses > & _action );

	virtual void visit( tracker::CGetBalanceAction & _action );

	virtual void visit( tracker::CValidateTransactionsAction & _action );

	virtual void visit( tracker::CConnectNodeAction & _action );

	virtual void visit( tracker::CTrackOriginAddressAction & _action );

	virtual void visit( tracker::CSynchronizationAction & _action );
private:
	tracker::TrackerResponses m_trackerResponses;
};

template<>
class CSetResponseVisitor< client::NodeResponses >
{
public:
	CSetResponseVisitor( client::NodeResponses const & _requestRespond );

	void visit( client::CSendTransactionAction & _action );

	void visit( client::CConnectAction & _action );

	void visit( CAction< client::NodeResponses > & _action );

	void visit( client::CSendBalanceInfoAction & _action );

	void visit( client::CPayLocalApplicationAction & _action );
private:

	client::NodeResponses m_requestResponse;
};


template<>
class CSetResponseVisitor< monitor::MonitorResponses >
{
public:
	CSetResponseVisitor( monitor::MonitorResponses const & _requestResponse );

	void visit( monitor::CConnectNodeAction & _action );
private:

	monitor::MonitorResponses m_requestResponse;
};

template<>
class CSetResponseVisitor< seed::SeedResponses >
{
public:
	CSetResponseVisitor( seed::SeedResponses const & _requestResponse );

	void visit( seed::CAcceptNodeAction & _action );

private:
	seed::SeedResponses m_requestResponse;
};


}

#endif
