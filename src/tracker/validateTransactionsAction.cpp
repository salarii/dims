#include "validateTransactionsAction.h"

#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/event.hpp>

#include "common/setResponseVisitor.h"
#include "common/commonResponses.h"

#include "validateTransactionsRequest.h"

#include "trackerEvents.h"
#include "transactionRecordManager.h"
#include "clientRequestsManager.h"
#include "trackerController.h"
#include "trackerFilters.h"
#include "trackerNodesManager.h"

namespace tracker
{

struct CNetworkPresent;

/*
when  transaction  bundle  is  approaching
generate request  to inform  every  node about it
remember all with  exception  of  node which send  bundle analyse  responses
validate transaction
send  double  spend
not ok
generate Ack  or  pass Ack
*/

struct COriginOfTransactionEvent : boost::statechart::event< COriginOfTransactionEvent >
{
};

struct CPasingTransactionEvent : boost::statechart::event< COriginOfTransactionEvent >
{
};

struct CSendAcceptBudle : boost::statechart::state< CSendAcceptBudle, CValidateTransactionsAction >
{
	CSendAcceptBudle( my_context ctx ) : my_base( ctx )
	{
		context< CValidateTransactionsAction >().setRequest( new CTransactionsStatusRequest( CBundleStatus::Ack, context< CValidateTransactionsAction >().getActionKey(), new CSpecificMediumFilter( context< CValidateTransactionsAction >().getInitiatingNode() ) ) );
	}

	boost::statechart::result react( common::CContinueEvent const & _continueEvent )
	{
		context< CValidateTransactionsAction >().setRequest( 0 );
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CContinueEvent >
	> reactions;
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

	boost::statechart::result react( common::CContinueEvent const & _continueEvent )
	{
		context< CValidateTransactionsAction >().setRequest( new common::CContinueReqest<TrackerResponses>( _continueEvent.m_keyId, new CComplexMediumFilter( m_participating ) ) );
		return discard_event();
	}

	boost::statechart::result react( common::CMessageResult const & _message )
	{
		m_participating.erase( _message.m_nodeIndicator );

	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CAckEvent >,
	boost::statechart::custom_reaction< common::CContinueEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >
	> reactions;

	std::set< uintptr_t > m_participating;
};

struct CPassBundleInvalidate : boost::statechart::state< CPassBundleInvalidate, CValidateTransactionsAction >
{
	CPassBundleInvalidate( my_context ctx ) : my_base( ctx )
	{
		context< CValidateTransactionsAction >().setRequest( new CTransactionsStatusRequest( CBundleStatus::NotValid, context< CValidateTransactionsAction >().getActionKey(), new CSpecificMediumFilter( context< CValidateTransactionsAction >().getInitiatingNode() ) ) );
	}

	boost::statechart::result react( common::CContinueEvent const & _continueEvent )
	{
		context< CValidateTransactionsAction >().setRequest( 0 );
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CContinueEvent >
	> reactions;
};

struct CPassBundle : boost::statechart::state< CPassBundle, CValidateTransactionsAction >
{
	CPassBundle( my_context ctx ) : my_base( ctx )
	{
		common::CMessageResult const* messageResult = dynamic_cast< common::CMessageResult const* >( simple_state::triggering_event() );

		assert( messageResult->m_message.m_header.m_payloadKind == common::CPayloadKind::Transactions );

		common::CMessage orginalMessage;

		if ( !common::CommunicationProtocol::unwindMessage( messageResult->m_message, orginalMessage, GetTime(), messageResult->m_pubKey ) );
		assert( !"service it somehow" );

		std::vector< CTransaction > & transactions = context< CValidateTransactionsAction >().acquireTransactions();
		convertPayload( orginalMessage, transactions );

		context< CValidateTransactionsAction >().setInitiatingNode( messageResult->m_nodeIndicator );

		context< CValidateTransactionsAction >().setRequest( new common::CAckRequest< TrackerResponses >( context< CValidateTransactionsAction >().getActionKey(), new CSpecificMediumFilter( messageResult->m_nodeIndicator ) ) );

		m_pubKey = messageResult->m_pubKey;
	}

	boost::statechart::result react( CValidationEvent const & _event )
	{
		// for  now  all or  nothing  philosophy

		if ( _event.m_invalidTransactionIndexes.empty() )
		{
			common::CMediumFilter< TrackerResponses > * filter = new CNodeExceptionFilter( context< CValidateTransactionsAction >().getInitiatingNode() );
			if ( CTrackerNodesManager::getInstance()->provideConnection( *filter ).empty() )
			{
				delete filter;
				transit<CSendAcceptBudle>();
			}
			else
			{
				CPassMessageRequest( context< CValidateTransactionsAction >().getMessage(), context< CValidateTransactionsAction >().getActionKey(), m_pubKey, filter );
				transit<CBroadcastBundle>();
			}
		}
		else
		{
			transit<CPassBundleInvalidate>();
		}
	}

	boost::statechart::result react( common::CContinueEvent const & _continueEvent )
	{
		//context< CValidateTransactionsAction >().setRequest( new common::CContinueReqest<TrackerResponses>( _continueEvent.m_keyId,  ) );
		return discard_event();
	}

	boost::statechart::result react( common::CAckPromptResult const & _ackPromptResult )
	{
		context< CValidateTransactionsAction >().setRequest(
					new CValidateTransactionsRequest( context< CValidateTransactionsAction >().getTransactions(), new CMediumClassFilter( common::CMediumKinds::Internal ) ) );
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CContinueEvent >,
	boost::statechart::custom_reaction< CValidationEvent >,
	boost::statechart::custom_reaction< common::CAckPromptResult >
	> reactions;

	CPubKey m_pubKey;
};

struct CPropagateBundle : boost::statechart::state< CPropagateBundle, CValidateTransactionsAction >
{
	CPropagateBundle( my_context ctx ) : my_base( ctx )
	{
		context< CValidateTransactionsAction >().setRequest(
					new CTransactionsPropagationRequest(
								context< CValidateTransactionsAction >().getTransactions(),
								context< CValidateTransactionsAction >().getActionKey(),
								new CMediumClassFilter( common::CMediumKinds::Trackers )
								)
					);
	}

	boost::statechart::result react( common::CContinueEvent const & _continueEvent )
	{
		context< CValidateTransactionsAction >().setRequest( new common::CContinueReqest<TrackerResponses>( _continueEvent.m_keyId, new CMediumClassFilter( common::CMediumKinds::Trackers ) ) );
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _event )
	{
		m_participating.insert( _event.m_nodePtr );
		context< CValidateTransactionsAction >().setRequest( new common::CContinueReqest<TrackerResponses>( context< CValidateTransactionsAction >().getActionKey(), new CMediumClassFilter( common::CMediumKinds::Trackers ) ) );
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CAckEvent >,
	boost::statechart::custom_reaction< common::CContinueEvent >
	> reactions;

	std::set< uintptr_t > m_participating;
};

struct CApproved : boost::statechart::state< CApproved, CValidateTransactionsAction >
{
	CApproved( my_context ctx ) : my_base( ctx )
	{

// instead of  calling  some  sort of request I will try to include  new transactions directly
/*		context< CValidateTransactionsAction >().m_request = 0;
		CTransactionRecordManager::getInstance()->addValidatedTransactionBundle(
			context< CValidateTransactionsAction >().m_transactions );

		CTransactionRecordManager::getInstance()->addTransactionsToStorage(
					context< CValidateTransactionsAction >().m_transactions );*/
	}

};

struct CRejected : boost::statechart::state< CRejected, CValidateTransactionsAction >
{
	CRejected( my_context ctx ) : my_base( ctx )
	{

		context< CValidateTransactionsAction >().setRequest( 0 );
	}
};

struct COriginInitial : boost::statechart::state< COriginInitial, CValidateTransactionsAction >
{

	COriginInitial( my_context ctx ) : my_base( ctx )
	{
		context< CValidateTransactionsAction >().setRequest(
				new CValidateTransactionsRequest( context< CValidateTransactionsAction >().getTransactions(), new CMediumClassFilter( common::CMediumKinds::Internal ) ) );
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
			CClientRequestsManager::getInstance()->setClientResponse( transaction.GetHash(), common::CTransactionAck( common::TransactionsStatus::Valdated, transaction ) );
		}

		if ( transactions.empty() )
			return transit< CRejected >();
		else
		{
			return CTrackerController::getInstance()->isConnected() ? transit< CPropagateBundle >() : transit< CApproved >();
		}
	}

	typedef boost::statechart::custom_reaction< CValidationEvent > reactions;
};


struct CInitial : boost::statechart::state< CInitial, CValidateTransactionsAction >
{
	CInitial( my_context ctx ) : my_base( ctx )
	{
	}

	boost::statechart::result react( COriginOfTransactionEvent const & _event )
	{
		return transit< COriginInitial >();
	}

	boost::statechart::result react( CPasingTransactionEvent const & _event )
	{
		return transit< CPassBundle >();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< COriginOfTransactionEvent >,
	boost::statechart::custom_reaction< CPasingTransactionEvent >
	> reactions;

};

CValidateTransactionsAction::CValidateTransactionsAction( std::vector< CTransaction > const & _transactions )
	: common::CAction< TrackerResponses >()
	, m_request( 0 )
	, m_transactions( _transactions )
{
	initiate();
	process_event( COriginOfTransactionEvent() );
}

CValidateTransactionsAction::CValidateTransactionsAction( common::CMessage const & _message )
{
	initiate();
	process_event( CPasingTransactionEvent() );
}

common::CRequest< TrackerResponses >*
CValidateTransactionsAction::execute()
{
	return m_request;
}

void
CValidateTransactionsAction::accept( common::CSetResponseVisitor< TrackerResponses > & _visitor )
{
	_visitor.visit( *this );
}

void
CValidateTransactionsAction::setRequest( common::CRequest< TrackerResponses > * _request )
{
	m_request = _request;
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
