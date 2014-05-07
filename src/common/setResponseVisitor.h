// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SET_RESPONSE_VISITOR_H
#define SET_RESPONSE_VISITOR_H

#include <boost/variant.hpp> 
#include "tracker/configureTrackerActionHandler.h"
#include "node/configureNodeActionHadler.h"

namespace tracker
{

class CGetBalanceAction;

class CValidateTransactionsAction;

}

namespace node
{

class CSendBalanceInfoAction;

class CSendTransactionAction;

class CConnectAction;
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
private:
	tracker::TrackerResponses m_trackerResponses;
};

template<>
class CSetResponseVisitor< node::NodeResponses >
{
public:
	CSetResponseVisitor( node::NodeResponses const & _requestRespond );

	void visit( node::CSendTransactionAction & _action );

	void visit( node::CConnectAction & _action );

	void visit( CAction< node::NodeResponses > & _action );

	void visit( node::CSendBalanceInfoAction & _action );
private:

	node::NodeResponses m_requestResponse;
};


}

#endif
