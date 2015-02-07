// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef VALIDATE_TRANSACTIONS_ACTION_H
#define VALIDATE_TRANSACTIONS_ACTION_H

#include <boost/statechart/state_machine.hpp>

#include "common/action.h"
#include "configureTrackerActionHandler.h"

namespace tracker
{

struct CInitial;
// extremely simplified,  no  confirmation message
class CValidateTransactionsAction : public common::CAction< TrackerResponses >,public boost::statechart::state_machine< CValidateTransactionsAction, CInitial >, public common::CCommunicationAction
{
public:
	CValidateTransactionsAction( std::vector< CTransaction > const & _transactions );

	CValidateTransactionsAction( uint256 const & _actionKey );

	virtual common::CRequest< TrackerResponses >* execute();

	virtual void accept( common::CSetResponseVisitor< TrackerResponses > & _visitor );

	void setRequest( common::CRequest< TrackerResponses > * _request );

	std::vector< CTransaction > const & getTransactions() const;

	std::vector< CTransaction > & acquireTransactions();

	void setTransactions( std::vector< CTransaction > const & _transactions );

	void setInitiatingNode( uintptr_t _initiatingNode );

	uintptr_t getInitiatingNode() const;

	void setMessage( common::CMessage const & _message );

	common::CMessage getMessage() const;
private:
	common::CRequest< TrackerResponses >* m_request;

	std::vector< CTransaction > m_transactions;// deque ??? because  removal  from this contaier may  happen

	uintptr_t m_initiatingNode;

	common::CMessage m_message;
};

}

#endif // VALIDATE_TRANSACTIONS_ACTION_H
