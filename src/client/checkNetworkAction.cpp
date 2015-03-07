// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "checkNetworkAction.h"

namespace client
{

struct CCheckTransactionStatus;

struct CInitiateNetworkCheck : boost::statechart::state< CInitiateNetworkCheck, CSendTransactionAction >
{
	CInitiateNetworkCheck( my_context ctx ) : my_base( ctx )
	{

	}
//  ack here
	boost::statechart::result react( common::CPending const & _pending )
	{
	}
/*
	boost::statechart::result react( CTransactionAckEvent const & _transactionSendAck )
	{
// todo, check status and validity of the transaction propagated
		if ( _transactionSendAck.m_status == common::TransactionsStatus::Validated )
		{
			CClientControl::getInstance()->addTransactionToModel( _transactionSendAck.m_transactionSend );
			context< CSendTransactionAction >().setValidatedTransactionHash( _transactionSendAck.m_transactionSend.GetHash() );
			return transit< CCheckTransactionStatus >();
		}
		else
		{
			context< CSendTransactionAction >().setRequest( 0 );
		}

		return discard_event();
	}
*/
	typedef boost::mpl::list<
	  boost::statechart::custom_reaction< common::CPending >
	//, boost::statechart::custom_reaction< CTransactionAckEvent >
	> reactions;
};

struct CCheckMonitors : boost::statechart::state< CCheckMonitors, CSendTransactionAction >
{
	CCheckMonitors( my_context ctx ) : my_base( ctx )
	{
	}

	boost::statechart::result react( common::CPending const & _pending )
	{
		context< CSendTransactionAction >().setRequest( new CInfoRequestContinue( _pending.m_token, new CSpecificMediumFilter( _pending.m_networkPtr ) ) );
		return discard_event();
	}

	typedef boost::mpl::list<
	  boost::statechart::custom_reaction<  common::CPending >
//	, boost::statechart::custom_reaction< common::CTransactionStatus >
	> reactions;
};

CCheckNetworkAction::CCheckNetworkAction( bool _autoDelete )
	: common::CAction< common::CClientTypes >( _autoDelete )
{
	initiate();
}

void
CCheckNetworkAction::accept( common::CSetResponseVisitor< common::CClientTypes > & _visitor )
{
	_visitor.visit( *this );
}
void
CCheckNetworkAction::reset()
{
	common::CAction< common::CClientTypes >::reset();
	initiate();
}

}
