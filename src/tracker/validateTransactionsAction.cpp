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

namespace tracker
{
// this is  conceptualy not clear  piece of  code
/*
 I see it  in this  fashion  in the future
initial
stand alone     network mode

*/

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
when  transaction  bundle  is  approach
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
// instead of  calling  some  sort of request I will try to include  new transaction directly
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
		if ( _event.m_valid )
			return transit< CApproved >();
		else
			return transit< CRejected >();
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
