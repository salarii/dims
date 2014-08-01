// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "connectAction.h"
#include "clientResponses.h"
#include "controlRequests.h"

#include "common/setResponseVisitor.h"
#include "common/commonEvents.h"

#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/event.hpp>

namespace client
{
const unsigned DnsAskLoopCounter = 10;
//
struct CClientUnconnected : boost::statechart::state< CClientUnconnected, CConnectAction >
{
	CClientUnconnected( my_context ctx ) : my_base( ctx )
	{
		context< CConnectAction >().setRequest( new CDnsInfoRequest() );
	}
	boost::statechart::result react( common::CContinueEvent const & _continueEvent )
	{
		m_counter--;
		context< CConnectAction >().setRequest( new common::CContinueReqest<NodeResponses>(uint256(), common::RequestKind::NetworkInfo ) );

		if ( !m_counter )
			context< CConnectAction >().setRequest( new CDnsInfoRequest() );
	}

	boost::statechart::result react( CDnsInfo const & _dnsInfo )
	{

	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CContinueEvent >,
	boost::statechart::custom_reaction< CDnsInfo >
	> reactions;

	unsigned int m_counter;
};

struct CRecognizeNetwork : boost::statechart::state< CRecognizeNetwork, CConnectAction >
{
	CRecognizeNetwork( my_context ctx ) : my_base( ctx )
	{
	}
};


/*
struct CUninitiated;

struct CSetupEvent : boost::statechart::event< CSetupEvent >
{
	CSetupEvent( bool _manual ):m_manual( _manual ){};
	bool const m_manual;
};

struct CResetEvent : boost::statechart::event< CResetEvent > {};
struct CGenerateRequest : boost::statechart::event< CGenerateRequest > {};

struct CConnectActionState : boost::statechart::state_machine< CConnectActionState, CUninitiated >
{
	CConnectActionState():m_request(0){};

	CRequest* m_request;
};

struct CManual : boost::statechart::simple_state< CManual, CConnectActionState >
{
	typedef boost::statechart::custom_reaction< CGenerateRequest > reactions;

	boost::statechart::result react( const CGenerateRequest & _generateRequest )
	{
		context< CConnectActionState >().m_request = new CTrackersInfoRequest( TrackerDescription );
	}

};
struct CAutomatic : boost::statechart::simple_state< CAutomatic, CConnectActionState >
{


};


struct CUninitiated : boost::statechart::simple_state< CUninitiated, CConnectActionState >
{
	typedef boost::statechart::custom_reaction< CSetupEvent > reactions;

	boost::statechart::result react( const CSetupEvent & _setupEvent )
	{
		if ( _setupEvent.m_manual )
			return transit< CManual >();
		else
			return transit< CAutomatic >();
	}
};

*/
CConnectAction::CConnectAction( State::Enum const _state )
:m_state( _state )
,m_actionStatus( common::ActionStatus::Unprepared )
{

}

void
CConnectAction::accept( common::CSetResponseVisitor< NodeResponses > & _visitor )
{
	_visitor.visit( *this );
}

common::CRequest< NodeResponses >*
CConnectAction::execute()
{

	// this  crap serves only  temporary
    // in final version it should be  some kind of state machine, fency style, because at the end we  can end up with quite complex code

	if ( m_state == State::Done )
		return 0;

	if ( m_actionStatus == common::ActionStatus::InProgress )
	{
		if ( m_state == State::Manual )
		{
			if ( m_trackerStats )
			{
				m_state = State::Done;

				CTrackerLocalRanking::getInstance()->addTracker( *m_trackerStats );
				return 0;
			}

		}
        else if ( m_token )
        {
			return new CInfoRequestContinue( *m_token, common::RequestKind::NetworkInfo );
        }
        else if ( m_error )
        {
			if ( *m_error == common::ErrorType::ServiceDenial )
            {
                // for now  simply repeat
                // it is not clear where should I place  code responsible  for such kind of errors
                // is seems that it is much  better  to pass it a little bit higher than  this
                // but since there is no method  to process errors at this point  I will react  where I am
                //
                return new CTrackersInfoRequest( TrackerDescription );
            }
        }
	}

	if ( m_state == State::Manual )
	{
		m_actionStatus = common::ActionStatus::InProgress;
		return new CTrackersInfoRequest( TrackerDescription );
	}
	else
	{
		// superfluous at  this point
	}

	return 0;
}

void
CConnectAction::reset()
{
	CAction::reset();

	m_token.reset();

	m_trackerStats.reset();

	m_monitorInfo.reset();

	m_error.reset();

}


void
CConnectAction::setTrackerInfo( boost::optional< common::CTrackerStats > const & _trackerStats )
{
	m_trackerStats = _trackerStats;
}

void
CConnectAction::setInProgressToken( boost::optional< uint256 > const & _trackerInfo )
{
	m_token = _trackerInfo;
}


void
CConnectAction::setMediumError( boost::optional< common::ErrorType::Enum > const & _error )
{
    m_error = _error;
}

void
CConnectAction::setRequest( common::CRequest< NodeResponses >* _request )
{
	m_request = _request;
}

}
