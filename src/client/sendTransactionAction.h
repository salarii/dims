// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SEND_TRANSACTION_ACTION_H
#define SEND_TRANSACTION_ACTION_H

#include <boost/optional.hpp>
#include <boost/statechart/state_machine.hpp>

#include "core.h"
#include "coins.h"

#include "common/action.h"
#include "common/request.h"
#include "common/transactionStatus.h"

namespace common
{

class CSetResponseVisitor;

}

namespace client
{

struct CPrepareAndSendTransaction;

class CSendTransactionAction : public common::CAction, public  boost::statechart::state_machine< CSendTransactionAction, CPrepareAndSendTransaction >
{
public:
	CSendTransactionAction( std::vector< std::pair< CKeyID, int64_t > > const & _outputs, std::vector< CSpendCoins > const & _sendCoins );

	void accept( common::CSetResponseVisitor & _visitor );

	void setProcessingTrackerPtr( 	uintptr_t _ptr );

	uintptr_t getProcessingTrackerPtr() const;

	void setTransaction( CTransaction const & _tx );

	CTransaction const & getTransaction() const;

	~CSendTransactionAction();
private:
	CTransaction m_transaction;

	uintptr_t m_processingTrackerPtr;
};

}

#endif // SEND_TRANSACTION_ACTION_H
