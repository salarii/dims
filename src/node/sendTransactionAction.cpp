// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/statechart/transition.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "sendTransactionAction.h"
#include "common/nodeMessages.h"
#include "common/setResponseVisitor.h"
#include "common/medium.h"
#include "clientFilters.h"
#include "clientRequests.h"

#include "configureNodeActionHadler.h"
#include "serialize.h"

using namespace common;

namespace client
{

struct CPrepareAndSendTransaction : boost::statechart::state< CPrepareAndSendTransaction, CSendTransactionAction >
{
	CPrepareAndSendTransaction( my_context ctx ) : my_base( ctx )
	{
		context< CSendTransactionAction >().setRequest( new CTransactionSendRequest( context< CSendTransactionAction >().getTransaction(), new CMediumClassFilter( RequestKind::Transaction, 1 ) ) );
	}
//  ack here
	boost::statechart::result react( common::CPending const & _pending )
	{
		context< CSendTransactionAction >().setRequest( new CInfoRequestContinue( _pending.m_token, new CSpecificMediumFilter( _pending.m_networkPtr ) ) );
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CPending >
	> reactions;
};
/*
get
transaction and hash


*/
struct CCheckTransactionStatus : boost::statechart::state< CCheckTransactionStatus, CSendTransactionAction >
{
	CCheckTransactionStatus( my_context ctx ) : my_base( ctx )
	{
	//	context< CSendTransactionAction >().setRequest( new CTransactionStatusRequest( m_token ) );
	}

	typedef boost::mpl::list<
//	boost::statechart::custom_reaction< common::CPending >
	> reactions;
};

CSendTransactionAction::CSendTransactionAction( const CTransaction & _transaction )
	: CAction()
	, m_transaction( _transaction )
	, m_actionStatus( common::ActionStatus::Unprepared )
{
	initiate();
}

void
CSendTransactionAction::accept( common::CSetResponseVisitor< NodeResponses > & _visitor )
{
	_visitor.visit( *this );
}


CRequest< NodeResponses > *
CSendTransactionAction::execute()
{
	return m_request;
}

void
CSendTransactionAction::setRequest( common::CRequest< NodeResponses > * _request )
{
	m_request = _request;
}

CTransaction const &
CSendTransactionAction::getTransaction() const
{
	return m_transaction;
}



CTransactionStatusRequest::CTransactionStatusRequest( uint256 const & _token, common::CMediumFilter< NodeResponses > * _medium )
	: common::CRequest< NodeResponses >( _medium )
	, m_token( _token )
{
}

void
CTransactionStatusRequest::accept( common::CMedium< NodeResponses > * _medium ) const
{
	_medium->add( this );
}

common::CMediumFilter< NodeResponses > *
CTransactionStatusRequest::getMediumFilter() const
{
	return common::CRequest< NodeResponses >::m_mediumFilter;
}

void
CTransactionSendRequest::accept( CMedium< NodeResponses > * _medium ) const
{
	_medium->add( this );
}

CTransactionSendRequest::CTransactionSendRequest( CTransaction const & _transaction, common::CMediumFilter< NodeResponses > * _medium )
	: common::CRequest< NodeResponses >( _medium )
	, m_transaction( _transaction )
{
}

common::CMediumFilter< NodeResponses > *
CTransactionSendRequest::getMediumFilter() const
{
	return common::CRequest< NodeResponses >::m_mediumFilter;
}


}
