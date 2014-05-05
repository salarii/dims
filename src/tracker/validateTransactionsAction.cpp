#include "validateTransactionsAction.h"

#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/event.hpp>

#include "common/setResponseVisitor.h"
#include "validateTransactionsRequest.h"

namespace tracker
{

struct CValidateTransactionsResultEvent : boost::statechart::event< CValidateTransactionsResultEvent >
{
};

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

struct CInitial : boost::statechart::simple_state< CInitial, CValidateTransactionsAction >
{
	typedef boost::statechart::custom_reaction< CValidateTransactionsResultEvent > reactions;

	CInitial()
	{
		context< CValidateTransactionsAction >().m_request =
				new CValidateTransactionsRequest( context< CValidateTransactionsAction >().m_transactions );
	}

	boost::statechart::result react( const CValidateTransactionsResultEvent & _event )
	{
		//if ( _event )
			return transit< CStandAlone >();
	//	else
	//		return transit< CAutomatic >();
	}
};

CValidateTransactionsAction::CValidateTransactionsAction( std::vector< CTransaction > const & _transactions )
	: common::CAction< TrackerResponses >()
	, m_request( 0 )
	, m_transactions( _transactions )
{
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
