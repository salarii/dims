#include "validateTransactionsAction.h"

#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/event.hpp>

namespace tracker
{

// states
// uninitiated
//propagate bundle
//ack bundle
// confirm bundle
// double spend
// invalid transaction bundle


struct CUninitiated;

struct CSetupEvent : boost::statechart::event< CSetupEvent >
{
};

struct CPropagationSummaryEvent : boost::statechart::event< CPropagationSummaryEvent >
{
};

struct CErrorEvent : boost::statechart::event< CErrorEvent >
{

};

struct CTransactionValidationStatemachine : boost::statechart::state_machine< CTransactionValidationStatemachine, CUninitiated >
{
	CTransactionValidationStatemachine(){};

};

struct CPropagate : boost::statechart::simple_state< CPropagate, CTransactionValidationStatemachine >
{
/*	typedef boost::statechart::custom_reaction< CGenerateRequest > reactions;

	boost::statechart::result react( const CGenerateRequest & _generateRequest )
	{
		context< CConnectActionState >().m_request = new CTrackersInfoRequest( TrackerDescription );
	}
*/
};
struct CAck : boost::statechart::simple_state< CAck, CTransactionValidationStatemachine >
{

};


struct CUninitiated : boost::statechart::simple_state< CUninitiated, CTransactionValidationStatemachine >
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

struct CConfirm
{
};


}
