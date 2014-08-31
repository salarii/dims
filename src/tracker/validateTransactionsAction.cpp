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

namespace tracker
{

struct CValidateTransactionsEvent : boost::statechart::event< CValidateTransactionsEvent >
{
};

struct CTransactionsKnownEvent : boost::statechart::event< CTransactionsKnownEvent >
{
};

struct CTransactionsAckEvent : boost::statechart::event< CTransactionsAckEvent >
{
};

struct CTransactionsDoublespendEvent : boost::statechart::event< CTransactionsDoublespendEvent >
{
};

struct CTransactionsNotOkEvent : boost::statechart::event< CTransactionsNotOkEvent >
{
};

struct CPropagationSummaryEvent : boost::statechart::event< CPropagationSummaryEvent >
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
remember all with  exception  of  node which send  bundle analyse  respponses
validate transaction
send  double  spend
not ok
generate Ack  or  pass Ack

*/
struct CApproved : boost::statechart::state< CApproved, CValidateTransactionsAction >
{
	CApproved( my_context ctx ) : my_base( ctx )
	{
// instead of  calling  some  sort of request I will try to include  new transactions directly
		context< CValidateTransactionsAction >().m_request = 0;
		CTransactionRecordManager::getInstance()->addValidatedTransactionBundle(
			context< CValidateTransactionsAction >().m_transactions );
	}

};
// not  ok  means
struct CRejected : boost::statechart::state< CRejected, CValidateTransactionsAction >
{
	CRejected( my_context ctx ) : my_base( ctx )
	{
// error should be indicated, at some point
		context< CValidateTransactionsAction >().m_request = 0;
	}
};

struct CInitial : boost::statechart::state< CInitial, CValidateTransactionsAction >
{
	typedef boost::statechart::custom_reaction< CValidationEvent > reactions;

	CInitial( my_context ctx ) : my_base( ctx )
	{
		context< CValidateTransactionsAction >().m_request =
				new CValidateTransactionsRequest( context< CValidateTransactionsAction >().m_transactions );
	}

	boost::statechart::result react( const CValidationEvent & _event )
	{
		std::vector< CTransaction > & transactions = context< CValidateTransactionsAction >().m_transactions;

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
			return transit< CApproved >();

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



}
