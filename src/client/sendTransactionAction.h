// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SEND_TRANSACTION_ACTION_H
#define SEND_TRANSACTION_ACTION_H

#include <boost/optional.hpp>

#include <boost/statechart/state_machine.hpp>

#include "common/action.h"
#include "common/request.h"
#include "configureClientActionHadler.h"
#include "core.h"
#include "common/transactionStatus.h"

namespace common
{

template < class _RequestResponses >
class CSetResponseVisitor;

}

namespace client
{

struct CPrepareAndSendTransaction;

class CSendTransactionAction : public common::CAction< common::CClientTypes >, public  boost::statechart::state_machine< CSendTransactionAction, CPrepareAndSendTransaction >
{
public:
	CSendTransactionAction( CTransaction const & _Transaction );

	void accept( common::CSetResponseVisitor< common::CClientTypes > & _visitor );

	void setProcessingTrackerPtr( 	uintptr_t _ptr );

	uintptr_t getProcessingTrackerPtr() const;

	void setTransaction( CTransaction const & _tx );

	CTransaction const & getTransaction() const;

	uint256 getInitialTransactionHash() const
	{
		return m_initialTransactionHash;
	}
private:
	CTransaction m_transaction;

	uintptr_t m_processingTrackerPtr;

	uint256 m_initialTransactionHash;
};

}

#endif // SEND_TRANSACTION_ACTION_H
