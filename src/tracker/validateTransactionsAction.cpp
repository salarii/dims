#include "validateTransactionsAction.h"

#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/event.hpp>

#include "common/setResponseVisitor.h"
#include "validateTransactionsRequest.h"

#include "trackerEvents.h"
#include "transactionRecordManager.h"
#include "clientRequestsManager.h"
#include "trackerController.h"
#include "trackerFilters.h"

namespace tracker
{

struct CValidateTransactionsEvent : boost::statechart::event< CValidateTransactionsEvent >
{
};

struct CTransactionsKnownEvent : boost::statechart::event< CTransactionsKnownEvent >
{
};

struct CTransactionsDoublespendEvent : boost::statechart::event< CTransactionsDoublespendEvent >
{
};

struct CTransactionsNotOkEvent : boost::statechart::event< CTransactionsNotOkEvent >
{
};

struct CTransactionsAckEvent : boost::statechart::event< CTransactionsAckEvent >
{
};

struct CErrorEvent : boost::statechart::event< CErrorEvent >
{
};

struct CNetworkPresent;

struct CNoTransaction : boost::statechart::state< CNoTransaction, CValidateTransactionsAction >
{
	CNoTransaction( my_context ctx ) : my_base( ctx )
	{
	}

	boost::statechart::result react( CValidateTransactionsEvent const & _validateTransactionsEvent )
	{
			return transit< CNetworkPresent >();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< CValidateTransactionsEvent >
	> reactions;
};

struct CNetworkPresent : boost::statechart::state< CNetworkPresent, CValidateTransactionsAction >
{
	CNetworkPresent( my_context ctx ) : my_base( ctx )
	{
		// choose nodes,  exclude  seeds, monitors, only  trackers  without one  who  sent bundle
	}

	boost::statechart::result react( CTransactionsKnownEvent const & _transactionsKnownEvent )
	{
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< CTransactionsKnownEvent >
	> reactions;
};

struct CStandAlone : boost::statechart::state< CStandAlone, CValidateTransactionsAction >
{
	CStandAlone( my_context ctx ) : my_base( ctx )
	{
		// validate internally
		// if  so  generate  acks ????
	}
};
/*
when  transaction  bundle  is  approaching
generate request  to inform  every  node about it
remember all with  exception  of  node which send  bundle analyse  responses
validate transaction
send  double  spend
not ok
generate Ack  or  pass Ack
*/

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

	boost::statechart::result react( CTransactionsAckEvent const & _event )
	{
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CAckEvent >,
	boost::statechart::custom_reaction< common::CContinueEvent >,
	boost::statechart::custom_reaction< CTransactionsAckEvent >
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

struct CInitial : boost::statechart::state< CInitial, CValidateTransactionsAction >
{
	typedef boost::statechart::custom_reaction< CValidationEvent > reactions;

	CInitial( my_context ctx ) : my_base( ctx )
	{
		context< CValidateTransactionsAction >().setRequest(
				new CValidateTransactionsRequest( context< CValidateTransactionsAction >().getTransactions() ) );
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
};

CValidateTransactionsAction::CValidateTransactionsAction( std::vector< CTransaction > const & _transactions )
	: common::CAction< TrackerResponses >()
	, m_request( 0 )
	, m_transactions( _transactions )
{
	initiate();
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

}
