// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef VALIDATE_TRANSACTIONS_ACTION_H
#define VALIDATE_TRANSACTIONS_ACTION_H

#include <boost/statechart/state_machine.hpp>

#include "common/action.h"

namespace tracker
{
struct CTransactionValidationStatemachine;

class CValidateTransactionsAction : public common::CAction
{
public:
	virtual common::CRequest* execute(){ return 0; }
private:
	CTransactionValidationStatemachine * m_transactionValidationState;
};

}

#endif // VALIDATE_TRANSACTIONS_ACTION_H
