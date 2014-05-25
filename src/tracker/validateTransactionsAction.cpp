#include "validateTransactionsAction.h"

#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/event.hpp>

#include "common/setResponseVisitor.h"
#include "validateTransactionsRequest.h"

#include "validateTransactionActionEvents.h"
#include "transactionRecordManager.h"

namespace tracker
{
// this is  conceptualy not clear  piece of  code
/*
 I see it  in this  fashion  in the future
initial
stand alone     network mode

*/
struct CPropagationSummaryEvent : boost::statechart::event< CPropagationSummaryEvent >
{
};

struct CErrorEvent : boost::statechart::event< CErrorEvent >
{
};

struct CNoTransaction : boost::statechart::simple_state< CNoTransaction, CValidateTransactionsAction >
{};

struct CNetworkPresent : boost::statechart::simple_state< CNetworkPresent, CValidateTransactionsAction >
{};

struct CStandAlone : boost::statechart::simple_state< CStandAlone, CValidateTransactionsAction >
{};

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
