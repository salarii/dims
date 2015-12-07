// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef VALIDATE_TRANSACTIONS_ACTION_H
#define VALIDATE_TRANSACTIONS_ACTION_H

#include <boost/statechart/state_machine.hpp>

#include "common/action.h"
#include "common/communicationProtocol.h"

namespace tracker
{

struct CInitial;

class CValidateTransactionsAction : public common::CAction,public boost::statechart::state_machine< CValidateTransactionsAction, CInitial >
{
public:
	CValidateTransactionsAction( std::vector< CTransaction > const & _transactions );

	CValidateTransactionsAction( uint256 const & _actionKey );

	virtual void accept( common::CSetResponseVisitor & _visitor );

	std::vector< CTransaction > const & getTransactions() const;

	std::vector< CTransaction > & acquireTransactions();

	void setTransactions( std::vector< CTransaction > const & _transactions );
public:
	CPubKey m_initiatingNodeKey;

	std::set< uint160 > m_passValidationTargets;

	common::CMessage m_transactionsMessage;

private:
	std::vector< CTransaction > m_transactions;// deque ??? because  removal  from this contaier may  happen
};

}

#endif // VALIDATE_TRANSACTIONS_ACTION_H
