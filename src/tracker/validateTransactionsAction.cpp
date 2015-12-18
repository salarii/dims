// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/event.hpp>

#include "common/setResponseVisitor.h"
#include "common/responses.h"
#include "common/analyseTransaction.h"
#include "common/authenticationProvider.h"

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

unsigned const PropagateWaitTime = 15000;

struct CNetworkPresent;
struct COriginInitial;
struct CPassBundle;
struct CRejected;
struct CPropagateBundle;
struct CPassBundleInvalidate;
struct CPassBundleValidate;


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
	boost::statechart::transition< CPasingTransactionEvent, CPassBundle >
	> reactions;
};

struct COriginInitial : boost::statechart::state< COriginInitial, CValidateTransactionsAction >
{

	COriginInitial( my_context ctx ) : my_base( ctx )
	{
		LogPrintf("validate transaction action: %p origin \n", &context< CValidateTransactionsAction >() );
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
					 , ids) )
			{
				invalidTransactions.push_back( transaction );
			}

			BOOST_FOREACH( CTxOut const & txOut, txOuts )
			{
				value += txOut.nValue;
			}
			if ( tracker::CController::getInstance()->getPrice() * ( transaction.vout.size() - 2 ) <= value )
				validTransactions.push_back( transaction );
			else
				invalidTransactions.push_back( transaction );
		}

		BOOST_FOREACH( CTransaction const & invalid, invalidTransactions )
		{
			CClientRequestsManager::getInstance()->setClientResponse( invalid.GetHash(), common::CTransactionAck( common::TransactionsStatus::Invalid, invalid ) );
		}

		context< CValidateTransactionsAction >().setTransactions( validTransactions );

		context< CValidateTransactionsAction >().forgetRequests();
		context< CValidateTransactionsAction >().addRequest(
				new CValidateTransactionsRequest( validTransactions, new CMediumClassFilter( common::CMediumKinds::Internal ) ) );
	}

	boost::statechart::result react( common::CValidationEvent const & _event )
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
			return CTrackerNodesManager::getInstance()->getNumberOfNetworkTrackers() > 0 ? transit< CPropagateBundle >() : transit< CApproved >();
		}
	}

	typedef boost::statechart::custom_reaction< common::CValidationEvent > reactions;
};

struct CPropagateBundle : boost::statechart::state< CPropagateBundle, CValidateTransactionsAction >
{
	CPropagateBundle( my_context ctx ) : my_base( ctx )
	{
		LogPrintf("validate transaction action: %p propagate bundle \n", &context< CValidateTransactionsAction >() );

		context< CValidateTransactionsAction >().forgetRequests();

		context< CValidateTransactionsAction >().addRequest(
				new common::CSendMessageRequest(
					common::CPayloadKind::Transactions
					, common::CTransactionBundle( context< CValidateTransactionsAction >().getTransactions() )
					, context< CValidateTransactionsAction >().getActionKey()
					, new CMediumClassFilter( common::CMediumKinds::Trackers ) ) );

		context< CValidateTransactionsAction >().addRequest(
					new common::CTimeEventRequest(
						 PropagateWaitTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

		std::set< common::CValidNodeInfo > networkTrackers
				= CTrackerNodesManager::getInstance()->getNetworkTrackers();

		BOOST_FOREACH( common::CValidNodeInfo const & validNodeInfo, networkTrackers )
		{
			if ( CTrackerNodesManager::getInstance()->isActiveNode( validNodeInfo.m_publicKey.GetID() ) )
				m_partners.insert( validNodeInfo.m_publicKey.GetID() );
		}
	}

	boost::statechart::result react( common::CAckEvent const & _event )
	{
		return discard_event();
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		if( _messageResult.m_message.m_header.m_payloadKind == common::CPayloadKind::StatusTransactions );
		{
			common::CMessage orginalMessage;

			std::set< CPubKey > participants;

			if ( !common::CommunicationProtocol::unwindMessageAndParticipants( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey, participants ) )
				assert( !"service it somehow" );

			common::CTransactionsBundleStatus status;
			convertPayload( orginalMessage, status );

			if ( status.m_status == (int)CBundleStatus::Validated )
			{
				BOOST_FOREACH( CPubKey const & _pubKey, participants )
				{
					m_partners.erase( _pubKey.GetID() );
				}
			}
			else
			{
				// not over yet, for now reject, in the future determine why it was rejected( inform  monitor about problem )
				BOOST_FOREACH( CTransaction const & transaction, context< CValidateTransactionsAction >().getTransactions() )
				{
					CClientRequestsManager::getInstance()->setClientResponse( transaction.GetHash(), common::CTransactionAck( common::TransactionsStatus::Invalid, transaction ) );
				}

				return transit< CRejected >();
			}

			if ( m_partners.empty() )
			{
				context< CValidateTransactionsAction >().addRequest(
							new common::CSendMessageRequest(
								common::CPayloadKind::StatusTransactions
								, common::CTransactionsBundleStatus( (int)CBundleStatus::Confirmed )
								, context< CValidateTransactionsAction >().getActionKey()
								, _messageResult.m_message.m_header.m_id
								, new CMediumClassFilter( common::CMediumKinds::Trackers ) ) );

				context< CValidateTransactionsAction >().forgetRequests();

				return transit< CApproved >();
			}
		}
		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CValidateTransactionsAction >().setExit();
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CAckEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CTimeEvent >
	> reactions;

	std::set< uint160 > m_partners;
};

struct CBroadcastBundle : boost::statechart::state< CBroadcastBundle, CValidateTransactionsAction >
{
	CBroadcastBundle( my_context ctx ) : my_base( ctx )
	{
		LogPrintf("validate transaction action: %p broadcast bundle \n", &context< CValidateTransactionsAction >() );

		context< CValidateTransactionsAction >().forgetRequests();

		context< CValidateTransactionsAction >().addRequest(
				new common::CSendMessageRequest(
					context< CValidateTransactionsAction >().m_transactionsMessage
					, context< CValidateTransactionsAction >().m_initiatingNodeKey
					, context< CValidateTransactionsAction >().getActionKey()
					, new CNodeExceptionFilter( common::CMediumKinds::Trackers, context< CValidateTransactionsAction >().m_initiatingNodeKey.GetID() ) ) );

		context< CValidateTransactionsAction >().addRequest(
					new common::CTimeEventRequest(
						PropagateWaitTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

		std::set< common::CValidNodeInfo > networkTrackers
				= CTrackerNodesManager::getInstance()->getNetworkTrackers();

		BOOST_FOREACH( common::CValidNodeInfo const & validNodeInfo, networkTrackers )
		{
			if ( CTrackerNodesManager::getInstance()->isActiveNode( validNodeInfo.m_publicKey.GetID() ) )
				m_partners.insert( validNodeInfo.m_publicKey.GetID() );
		}

		m_partners.erase( context< CValidateTransactionsAction >().m_initiatingNodeKey.GetID() );

		m_known.insert(context< CValidateTransactionsAction >().m_initiatingNodeKey.GetID());
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CValidateTransactionsAction >().m_passValidationTargets = m_partners;  // burden, clean it
		return transit< CPassBundleValidate >();
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		std::set< CPubKey > nodes;

		context< CValidateTransactionsAction >().addRequest(
					new common::CAckRequest(
						  context< CValidateTransactionsAction >().getActionKey()
						, _messageResult.m_message.m_header.m_id
						, new CByKeyMediumFilter( _messageResult.m_pubKey ) ) );

		if ( _messageResult.m_message.m_header.m_payloadKind == common::CPayloadKind::Transactions )
		{
			common::CMessage orginalMessage;

			if ( !common::CommunicationProtocol::unwindMessageAndParticipants( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey, nodes ) )
				assert( !"service it somehow" );

			BOOST_FOREACH( CPubKey const & key, nodes )
			{
				m_known.insert(key.GetID());
			}

			context< CValidateTransactionsAction >().addRequest(
					new common::CSendMessageRequest(
						common::CPayloadKind::StatusTransactions
						, common::CTransactionsBundleStatus( (int)CBundleStatus::Known )
						, context< CValidateTransactionsAction >().getActionKey()
						, new CByKeyMediumFilter( _messageResult.m_pubKey ) ) );
		}
		else if( _messageResult.m_message.m_header.m_payloadKind == common::CPayloadKind::StatusTransactions )
		{
			common::CMessage orginalMessage;

			if ( !common::CommunicationProtocol::unwindMessageAndParticipants( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey, nodes ) )
				assert( !"service it somehow" );

			common::CTransactionsBundleStatus status;

			convertPayload( orginalMessage, status );

			BOOST_FOREACH( CPubKey const & key, nodes )
			{
				if ( status.m_status == CBundleStatus::Validated )
				{
					m_validate.insert(key.GetID());
				}
				else if ( status.m_status == CBundleStatus::Known )
				{
					m_known.insert(key.GetID());
				}
			}
		}

		std::set<uint160> comparePartners = m_validate;

		comparePartners.insert(m_known.begin(), m_known.end());

		if ( comparePartners == m_partners )
		{
			context< CValidateTransactionsAction >().m_passValidationTargets = m_known;
			return transit< CPassBundleValidate >();
		}

		return discard_event();
	}

	boost::statechart::result react( common::CNoMedium const & _noMedium )
	{
		context< CValidateTransactionsAction >().m_passValidationTargets = m_known;
		return transit< CPassBundleValidate >();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CAckEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CNoMedium >,
	boost::statechart::custom_reaction< common::CTimeEvent >
	> reactions;

	std::set< uint160 > m_known;
	std::set< uint160 > m_validate;
	std::set< uint160 > m_partners;
};

struct CPassBundle : boost::statechart::state< CPassBundle, CValidateTransactionsAction >
{
	CPassBundle( my_context ctx ) : my_base( ctx )
	{
		LogPrintf("validate transaction action: %p pass bundle \n", &context< CValidateTransactionsAction >() );
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		if ( _messageResult.m_message.m_header.m_payloadKind == common::CPayloadKind::Transactions );
		{
			common::CMessage orginalMessage;

			if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
				assert( !"service it somehow" );

			common::CTransactionBundle transactionBundle;

			convertPayload( orginalMessage, transactionBundle );

			context< CValidateTransactionsAction >().setTransactions( transactionBundle.m_transactions );

			context< CValidateTransactionsAction >().m_initiatingNodeKey = _messageResult.m_pubKey;

			context< CValidateTransactionsAction >().m_transactionsMessage = _messageResult.m_message;

			context< CValidateTransactionsAction >().forgetRequests();

			context< CValidateTransactionsAction >().addRequest(
						new common::CAckRequest(
							context< CValidateTransactionsAction >().getActionKey()
							, _messageResult.m_message.m_header.m_id
							, new CByKeyMediumFilter( _messageResult.m_pubKey ) ) );

			context< CValidateTransactionsAction >().addRequest(
						new CValidateTransactionsRequest(
							context< CValidateTransactionsAction >().getTransactions()
							, new CMediumClassFilter( common::CMediumKinds::Internal ) ) );

		}
		return discard_event();
	}
	boost::statechart::result react( common::CValidationEvent const & _event )
	{
		// for  now  all or  nothing  philosophy
		if ( _event.m_invalidTransactionIndexes.empty() )
		{
				return transit<CBroadcastBundle>();
		}
		else
		{
			return transit<CPassBundleInvalidate>();
		}
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CValidationEvent >
	> reactions;

};

struct CPassBundleValidate : boost::statechart::state< CPassBundleValidate, CValidateTransactionsAction >
{
	CPassBundleValidate( my_context ctx ) : my_base( ctx )
	{
		LogPrintf("validate transaction action: %p pass bundle validate \n", &context< CValidateTransactionsAction >() );

		context< CValidateTransactionsAction >().addRequest(
				new common::CSendMessageRequest(
					common::CPayloadKind::StatusTransactions
					, common::CTransactionsBundleStatus( (int)CBundleStatus::Validated )
					, context< CValidateTransactionsAction >().getActionKey()
					, new CComplexMediumFilter( context< CValidateTransactionsAction >().m_passValidationTargets ) ) );

		context< CValidateTransactionsAction >().addRequest(
					new common::CTimeEventRequest(
						PropagateWaitTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		std::set< CPubKey > nodes;

		context< CValidateTransactionsAction >().addRequest(
					new common::CAckRequest(
						  context< CValidateTransactionsAction >().getActionKey()
						, _messageResult.m_message.m_header.m_id
						, new CByKeyMediumFilter( _messageResult.m_pubKey ) ) );

		if ( _messageResult.m_message.m_header.m_payloadKind == common::CPayloadKind::StatusTransactions )
		{
			common::CMessage orginalMessage;

			if ( !common::CommunicationProtocol::unwindMessageAndParticipants( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey, nodes ) )
				assert( !"service it somehow" );

			BOOST_FOREACH( CPubKey const & key, nodes )
			{
				m_exceptions.insert(key.GetID());
			}

			common::CTransactionsBundleStatus status;

			convertPayload( orginalMessage, status );

			if ( status.m_status == CBundleStatus::Confirmed )
			{
				context< CValidateTransactionsAction >().addRequest(
							new common::CSendMessageRequest(
								_messageResult.m_message
								, _messageResult.m_pubKey
								, context< CValidateTransactionsAction >().getActionKey()
								, new CNodeExceptionFilter( common::CMediumKinds::Trackers, m_exceptions ) ) );

				context< CValidateTransactionsAction >().forgetRequests();

				return transit<CApproved>();
			}

		}
		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		//something  wrong, but  forget  about  that for now
		context< CValidateTransactionsAction >().forgetRequests();
		context< CValidateTransactionsAction >().setExit();
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		return discard_event();
	}

	boost::statechart::result react( common::CNoMedium const & _noMedium )
	{
		assert(!"problem");
		context< CValidateTransactionsAction >().forgetRequests();
		context< CValidateTransactionsAction >().setExit();
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CAckEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CNoMedium >,
	boost::statechart::custom_reaction< common::CTimeEvent >
	> reactions;

	std::set< uint160 > m_exceptions;
};

struct CPassBundleInvalidate : boost::statechart::state< CPassBundleInvalidate, CValidateTransactionsAction >
{
	CPassBundleInvalidate( my_context ctx ) : my_base( ctx )
	{
		LogPrintf("validate transaction action: %p pass bundle invalidate \n", &context< CValidateTransactionsAction >() );

		context< CValidateTransactionsAction >().forgetRequests();

		context< CValidateTransactionsAction >().addRequest(
				new common::CSendMessageRequest(
					common::CPayloadKind::StatusTransactions
					, common::CTransactionsBundleStatus( (int)CBundleStatus::NotValid )
					, context< CValidateTransactionsAction >().getActionKey()
					, new CByKeyMediumFilter( context< CValidateTransactionsAction >().m_initiatingNodeKey ) ) );

				context< CValidateTransactionsAction >().setExit(); // too optimistic
	}
};

struct CApproved : boost::statechart::state< CApproved, CValidateTransactionsAction >
{
	CApproved( my_context ctx ) : my_base( ctx )
	{
		LogPrintf("validate transaction action: %p approved \n", &context< CValidateTransactionsAction >() );

		// make real check  against storage ??
		CTransactionRecordManager::getInstance()->addValidatedTransactionBundle(
			context< CValidateTransactionsAction >().getTransactions() );

		CTransactionRecordManager::getInstance()->addTransactionsToStorage(
					context< CValidateTransactionsAction >().getTransactions() );

		std::vector< CTransaction > transactions = context< CValidateTransactionsAction >().getTransactions();

		BOOST_FOREACH( CTransaction const & transaction, transactions )
		{
			common::findSelfCoinsAndAddToWallet( transaction );
		}

		context< CValidateTransactionsAction >().forgetRequests();

		context< CValidateTransactionsAction >().addRequest(
				new common::CSendMessageRequest(
					common::CPayloadKind::Transactions
					, common::CTransactionBundle( context< CValidateTransactionsAction >().getTransactions() )
					, context< CValidateTransactionsAction >().getActionKey()
					, new CMediumClassFilter( common::CMediumKinds::Monitors ) ) );
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		context< CValidateTransactionsAction >().setExit();
		return discard_event();
	}

	boost::statechart::result react( common::CNoMedium const & _noMedium )
	{
		context< CValidateTransactionsAction >().setExit();
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CAckEvent >,
	boost::statechart::custom_reaction< common::CNoMedium >
	> reactions;

};

struct CRejected : boost::statechart::state< CRejected, CValidateTransactionsAction >
{
	CRejected( my_context ctx ) : my_base( ctx )
	{
		context< CValidateTransactionsAction >().forgetRequests();
		context< CValidateTransactionsAction >().setExit();
	}
};

CValidateTransactionsAction::CValidateTransactionsAction( std::vector< CTransaction > const & _transactions )
	: common::CAction()
	, m_transactions( _transactions )
{
	LogPrintf("validate transaction action: %p origin \n", this );

	initiate();
	process_event( COriginOfTransactionEvent() );
}

CValidateTransactionsAction::CValidateTransactionsAction( uint256 const & _actionKey )
	: common::CAction( _actionKey )
{
	LogPrintf("validate transaction action: %p pass bundle \n", this );
	initiate();
	process_event( CPasingTransactionEvent() );
}

void
CValidateTransactionsAction::accept( common::CSetResponseVisitor & _visitor )
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

}
