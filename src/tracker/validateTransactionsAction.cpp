#include "validateTransactionsAction.h"

#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/event.hpp>

#include "common/setResponseVisitor.h"

namespace tracker
{

struct CSetupEvent : boost::statechart::event< CSetupEvent >
{
};

struct CValidateTransactionsEvent
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

struct CUninitiated : boost::statechart::simple_state< CUninitiated, CValidateTransactionsAction >
{
/*	typedef boost::statechart::custom_reaction< CSetupEvent > reactions;

	boost::statechart::result react( const CSetupEvent & _setupEvent )
	{
		if ( _setupEvent.m_manual )
			return transit< CManual >();
		else
			return transit< CAutomatic >();
	}*/
};

CValidateTransactionsAction::CValidateTransactionsAction( std::vector< CTransaction > const & _transactions )
	:common::CAction< TrackerResponses >()
{
}

void
CValidateTransactionsAction::accept( common::CSetResponseVisitor< TrackerResponses > & _visitor )
{
	_visitor.visit( *this );
}



}
