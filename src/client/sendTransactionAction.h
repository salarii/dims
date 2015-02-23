// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SEND_TRANSACTION_ACTION_H
#define SEND_TRANSACTION_ACTION_H

#include <boost/optional.hpp>

#include <boost/statechart/state_machine.hpp>

#include "common/action.h"
#include "common/request.h"
#include "configureNodeActionHadler.h"
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

class CSendTransactionAction : public common::CAction< NodeResponses >, public  boost::statechart::state_machine< CSendTransactionAction, CPrepareAndSendTransaction >
{
public:
	CSendTransactionAction( const CTransaction & _Transaction );

	void accept( common::CSetResponseVisitor< NodeResponses > & _visitor );

	common::CRequest< NodeResponses > * getRequest() const;

	void setRequest( common::CRequest< NodeResponses > * _request );

	CTransaction const & getTransaction() const;

	void setProcessingTrackerPtr( 	uintptr_t _ptr );

	uintptr_t getProcessingTrackerPtr() const;

	void setValidatedTransactionHash( uint256 _hash );

	uint256 getValidatedTransactionHash() const;
private:
	CTransaction m_transaction;
	
	common::CRequest< NodeResponses > * m_request;

	uintptr_t m_processingTrackerPtr;

	uint256 m_validatedTransactionHash;
};

}

#endif // SEND_TRANSACTION_ACTION_H
