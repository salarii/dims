// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/event.hpp>

#include "common/setResponseVisitor.h"
#include "common/commonResponses.h"
#include "common/analyseTransaction.h"

#include "tracker/validateTransactionsAction.h"
#include "tracker/events.h"
#include "tracker/transactionRecordManager.h"
#include "tracker/clientRequestsManager.h"
#include "tracker/controller.h"
#include "tracker/filters.h"
#include "tracker/trackerNodesManager.h"
#include "tracker/requests.h"

#include "wallet.h"
/*
when  transaction  bundle  is  approaching
generate request  to inform  every  node about it
remember all with  exception  of  node which send  bundle analyse  responses
validate transaction
send  double  spend
not ok
generate Ack  or  pass Ack
*/

namespace tracker
{

struct CNetworkPresent;
struct COriginInitial;
struct CPassBundle;
struct CRejected;
struct CPropagateBundle;
struct CPassBundleInvalidate;

struct COriginOfTransactionEvent : boost::statechart::event< COriginOfTransactionEvent >
{
};

struct CPasingTransactionEvent : boost::statechart::event< CPasingTransactionEvent >
{
};

struct CApproved;

struct CInitial : boost::statechart::state< CInitial, CValidateTransactionsAction >
{
	CInitial( my_context ctx ) : my_base( ctx )
	{
	}

	typedef boost::mpl::list<
	boost::statechart::transition< COriginOfTransactionEvent, COriginInitial >,
	boost::statechart::transition< common::CMessageResult, CPassBundle >
	> reactions;
};

struct COriginInitial : boost::statechart::state< COriginInitial, CValidateTransactionsAction >
{

	COriginInitial( my_context ctx ) : my_base( ctx )
	{
		/* check fee, move it to some other place??? */
		std::vector< CTransaction > validTransactions;
		std::vector< CTransaction > invalidTransactions;

		std::vector < CTxOut > txOuts;
		std::vector< unsigned int > ids;
		unsigned int value = 0;
		BOOST_FOREACH( CTransaction const & transaction, context< CValidateTransactionsAction >().getTransactions() )
		{

			if ( !common::findOutputInTransaction(
					 transaction
					 , common::CAuthenticationProvider::getInstance()->getMyKey().GetID()
					 , txOuts
					 ,ids) )
			{
				invalidTransactions.push_back( transaction );
			}

			BOOST_FOREACH( CTxOut const & txOut, txOuts )
			{
				value += txOut.nValue;
			}
			if ( tracker::CController::getInstance()->getPrice() <= value )
				validTransactions.push_back( transaction );
			else
				invalidTransactions.push_back( transaction );
		}

		BOOST_FOREACH( CTransaction const & invalid, invalidTransactions )
		{
			CClientRequestsManager::getInstance()->setClientResponse( invalid.GetHash(), common::CTransactionAck( common::TransactionsStatus::Invalid, invalid ) );
		}

		context< CValidateTransactionsAction >().setTransactions( validTransactions );

		context< CValidateTransactionsAction >().dropRequests();
		context< CValidateTransactionsAction >().addRequest(
				new CValidateTransactionsRequest( validTransactions, new CMediumClassFilter( common::CMediumKinds::Internal ) ) );
	}

	boost::statechart::result react( CValidationEvent const & _event )
	{
		std::vector< CTransaction > & transactions = context< CValidateTransactionsAction >().acquireTransactions();

		BOOST_FOREACH( unsigned int index, _event.m_invalidTransactionIndexes )
		{
			CClientRequestsManager::getInstance()->setClientResponse( transactions.at( index ).GetHash(), common::CTransactionAck( common::TransactionsStatus::Invalid, transactions.at( index ) ) );
		}

		//bit  faster  removal
		if ( !_event.m_invalidTransactionIndexes.empty() )
		{
			std::vector< CTransaction >::iterator last = transactions.begin() + _event.m_invalidTransactionIndexes.at(0);
			std::vector< CTransaction >::iterator previous = last;
			for ( unsigned int i = 1; i < _event.m_invalidTransactionIndexes.size(); ++i )
			{
				std::vector< CTransaction >::iterator next = transactions.begin() + (unsigned int)_event.m_invalidTransactionIndexes[ i ];
				unsigned int distance = std::distance( previous, next );
				if ( distance > 1 )
				{
					std::copy( previous + 1, next, last );

					last = last + distance - 1;
				}

				previous = next;
			}

			if ( previous + 1 != transactions.end() )
			{
				std::copy( previous + 1, transactions.end(), last );
				last += std::distance( previous + 1, transactions.end() );

			}
			transactions.resize( std::distance( transactions.begin(), last ) );
		}

		BOOST_FOREACH( CTransaction const & transaction, transactions )
		{
			CClientRequestsManager::getInstance()->setClientResponse( transaction.GetHash(), common::CTransactionAck( common::TransactionsStatus::Validated, transaction ) );
		}

		if ( transactions.empty() )
			return transit< CRejected >();
		else
		{
			return CTrackerNodesManager::getInstance()->getNumberOfTrackers() > 0 ? transit< CPropagateBundle >() : transit< CApproved >();
		}
	}

	typedef boost::statechart::custom_reaction< CValidationEvent > reactions;
};

struct CPropagateBundle : boost::statechart::state< CPropagateBundle, CValidateTransactionsAction >
{
	CPropagateBundle( my_context ctx ) : my_base( ctx ), m_totalWaitTime( 30 )
	{
		context< CValidateTransactionsAction >().dropRequests();
		context< CValidateTransactionsAction >().addRequest(
					new CTransactionsPropagationRequest(
								context< CValidateTransactionsAction >().getTransactions(),
								context< CValidateTransactionsAction >().getActionKey(),
								new CMediumClassFilter( common::CMediumKinds::Trackers )
								)
					);

		m_time = GetTime();
	}

	boost::statechart::result react( common::CAckEvent const & _event )
	{
		//not needed for the time being
		m_participating.insert( _event.m_nodePtr );
		context< CValidateTransactionsAction >().dropRequests();
		return discard_event();
	}

	boost::statechart::result react( common::CMessageResult const & _event )
	{
		assert( _event.m_message.m_header.m_payloadKind == common::CPayloadKind::StatusTransactions );

		common::CMessage orginalMessage;

		if ( !common::CommunicationProtocol::unwindMessage( _event.m_message, orginalMessage, GetTime(), _event.m_pubKey ) )
			assert( !"service it somehow" );

		common::CTransactionsBundleStatus status;
		convertPayload( orginalMessage, status );

		//  do  nothing with  result  right  now


		m_participating.erase( _event.m_nodeIndicator  );
		if ( m_participating.empty() )
		{
			return transit< CApproved >();
		}

		context< CValidateTransactionsAction >().dropRequests();
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CAckEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >
	> reactions;

	std::set< uintptr_t > m_participating;

	uint64_t m_time;

	uint64_t const m_totalWaitTime;
};

struct CSendAcceptBudle : boost::statechart::state< CSendAcceptBudle, CValidateTransactionsAction >
{
	CSendAcceptBudle( my_context ctx ) : my_base( ctx )
	{
		context< CValidateTransactionsAction >().dropRequests();
		context< CValidateTransactionsAction >().addRequest( new CTransactionsStatusRequest( CBundleStatus::Ack, context< CValidateTransactionsAction >().getActionKey(), new CSpecificMediumFilter( context< CValidateTransactionsAction >().getInitiatingNode() ) ) );
	}

};


struct CBroadcastBundle : boost::statechart::state< CBroadcastBundle, CValidateTransactionsAction >
{
	CBroadcastBundle( my_context ctx ) : my_base( ctx )
	{
	}
	//ugly  check
	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		return discard_event();
	}

	boost::statechart::result react( common::CMessageResult const & _message )
	{
		m_participating.erase( _message.m_nodeIndicator );
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CAckEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >
	> reactions;

	std::set< uintptr_t > m_participating;
};

struct CPassBundle : boost::statechart::state< CPassBundle, CValidateTransactionsAction >
{
	CPassBundle( my_context ctx ) : my_base( ctx )
	{
		common::CMessageResult const* messageResult = dynamic_cast< common::CMessageResult const* >( simple_state::triggering_event() );

		assert( messageResult->m_message.m_header.m_payloadKind == common::CPayloadKind::Transactions );

		common::CMessage orginalMessage;

		if ( !common::CommunicationProtocol::unwindMessage( messageResult->m_message, orginalMessage, GetTime(), messageResult->m_pubKey ) )
			assert( !"service it somehow" );

		common::CTransactionBundle transactionBundle;

		convertPayload( orginalMessage, transactionBundle );

		context< CValidateTransactionsAction >().setTransactions( transactionBundle.m_transactions );

		context< CValidateTransactionsAction >().setInitiatingNode( messageResult->m_nodeIndicator );

		context< CValidateTransactionsAction >().dropRequests();

		context< CValidateTransactionsAction >().addRequest(
					new common::CAckRequest< common::CTrackerTypes >(
						  context< CValidateTransactionsAction >().getActionKey()
						, messageResult->m_message.m_header.m_id
						, new CSpecificMediumFilter( messageResult->m_nodeIndicator ) ) );

		context< CValidateTransactionsAction >().addRequest(
					new CValidateTransactionsRequest( context< CValidateTransactionsAction >().getTransactions(), new CMediumClassFilter( common::CMediumKinds::Internal ) ) );

		m_pubKey = messageResult->m_pubKey;
	}

	boost::statechart::result react( CValidationEvent const & _event )
	{
		// for  now  all or  nothing  philosophy

		if ( _event.m_invalidTransactionIndexes.empty() )
		{
			common::CTrackerMediumFilter * filter = new CNodeExceptionFilter( context< CValidateTransactionsAction >().getInitiatingNode() );
			if ( CTrackerNodesManager::getInstance()->provideConnection( *filter ).empty() )
			{
				delete filter;
				return transit<CSendAcceptBudle>();
			}
			else
			{
				CPassMessageRequest( context< CValidateTransactionsAction >().getMessage(), context< CValidateTransactionsAction >().getActionKey(), m_pubKey, filter );
				return transit<CBroadcastBundle>();
			}
		}
		else
		{
			return transit<CPassBundleInvalidate>();
		}
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< CValidationEvent >
	> reactions;

	CPubKey m_pubKey;
};

struct CPassBundleInvalidate : boost::statechart::state< CPassBundleInvalidate, CValidateTransactionsAction >
{
	CPassBundleInvalidate( my_context ctx ) : my_base( ctx )
	{
		context< CValidateTransactionsAction >().dropRequests();
		context< CValidateTransactionsAction >().addRequest( new CTransactionsStatusRequest( CBundleStatus::NotValid, context< CValidateTransactionsAction >().getActionKey(), new CSpecificMediumFilter( context< CValidateTransactionsAction >().getInitiatingNode() ) ) );
	}
};

struct CApproved : boost::statechart::state< CApproved, CValidateTransactionsAction >
{
	CApproved( my_context ctx ) : my_base( ctx )
	{
		CTransactionRecordManager::getInstance()->addValidatedTransactionBundle(
			context< CValidateTransactionsAction >().getTransactions() );

		CTransactionRecordManager::getInstance()->addTransactionsToStorage(
					context< CValidateTransactionsAction >().getTransactions() );

		std::vector< CTransaction > transactions = context< CValidateTransactionsAction >().getTransactions();

		BOOST_FOREACH( CTransaction const & transaction, transactions )
		{
			common::findSelfCoinsAndAddToWallet( transaction );
		}

		context< CValidateTransactionsAction >().dropRequests();
		context< CValidateTransactionsAction >().addRequest(
					new CTransactionsPropagationRequest(
								context< CValidateTransactionsAction >().getTransactions(),
								context< CValidateTransactionsAction >().getActionKey(),
								new CMediumClassFilter( common::CMediumKinds::Monitors )
								)
					);

	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		context< CValidateTransactionsAction >().setExit();
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;

};

struct CRejected : boost::statechart::state< CRejected, CValidateTransactionsAction >
{
	CRejected( my_context ctx ) : my_base( ctx )
	{
		context< CValidateTransactionsAction >().dropRequests();
	}
};

CValidateTransactionsAction::CValidateTransactionsAction( std::vector< CTransaction > const & _transactions )
	: common::CAction< common::CTrackerTypes >()
	, m_transactions( _transactions )
{
	initiate();
	process_event( COriginOfTransactionEvent() );
}

CValidateTransactionsAction::CValidateTransactionsAction( uint256 const & _actionKey )
	: common::CAction< common::CTrackerTypes >( _actionKey )
{
	initiate();
}

void
CValidateTransactionsAction::accept( common::CSetResponseVisitor< common::CTrackerTypes > & _visitor )
{
	_visitor.visit( *this );
}

std::vector< CTransaction > const &
CValidateTransactionsAction::getTransactions() const
{
	return m_transactions;
}

std::vector< CTransaction > &
CValidateTransactionsAction::acquireTransactions()
{
	return m_transactions;
}

void
CValidateTransactionsAction::setTransactions( std::vector< CTransaction > const & _transactions )
{
	m_transactions = _transactions;
}

void
CValidateTransactionsAction::setInitiatingNode( uintptr_t _initiatingNode )
{
	m_initiatingNode = _initiatingNode;
}

uintptr_t
CValidateTransactionsAction::getInitiatingNode() const
{
	return m_initiatingNode;
}

void
CValidateTransactionsAction::setMessage( common::CMessage const & _message )
{
	m_message = _message;
}

common::CMessage
CValidateTransactionsAction::getMessage() const
{
	return m_message;
}

}
