// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "addTrackerAction.h"
#include "common/setResponseVisitor.h"
#include "common/commonEvents.h"
#include "common/authenticationProvider.h"
#include "common/mediumRequests.h"

#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

namespace monitor
{


struct CBothUnidentifiedConnected;

struct CUninitiated : boost::statechart::simple_state< CUninitiated, CAddTrackerAction >
{
	typedef boost::mpl::list<
	boost::statechart::transition< common::CSwitchToConnectedEvent, CBothUnidentifiedConnected >
	> reactions;
};


template < class Parent >
void
createIdentifyResponse( Parent & parent )
{
	uint256 hash = Hash( &parent.getPayload().front(), &parent.getPayload().back() );

	std::vector< unsigned char > signedHash;
	common::CAuthenticationProvider::getInstance()->sign( hash, signedHash );

	parent.setRequest( new common::CIdentifyResponse< MonitorResponses >( parent.getMediumKind(), signedHash, common::CAuthenticationProvider::getInstance()->getMyKey(), parent.getPayload(), parent.getActionKey() ) );
}


struct CPairIdentifiedConnected : boost::statechart::state< CPairIdentifiedConnected, CAddTrackerAction >
{
	CPairIdentifiedConnected( my_context ctx ) : my_base( ctx )
	{
		context< CAddTrackerAction >().setRequest( 0 );
	}
};

struct CBothUnidentifiedConnected : boost::statechart::state< CBothUnidentifiedConnected, CAddTrackerAction >
{
	CBothUnidentifiedConnected( my_context ctx ) : my_base( ctx )
	{
		common::CIntroduceEvent const* requestedEvent = dynamic_cast< common::CIntroduceEvent const* >( simple_state::triggering_event() );

		createIdentifyResponse( context< CAddTrackerAction >() );

	}

	boost::statechart::result react( const common::CContinueEvent & _continueEvent )
	{
		context< CAddTrackerAction >().setRequest( new common::CContinueReqest< MonitorResponses >( _continueEvent.m_keyId, context< CAddTrackerAction >().getMediumKind() ) );
	}

	typedef boost::mpl::list<
	boost::statechart::transition< common::CIntroduceEvent, CPairIdentifiedConnected >,
	boost::statechart::custom_reaction< common::CContinueEvent >
	> reactions;
};


CAddTrackerAction::CAddTrackerAction( std::vector< unsigned char > const & _payload, unsigned int _mediumKind )
	: m_request( 0 )
	, m_payload( _payload )
	, m_mediumKind( _mediumKind )
{
	initiate();
	process_event( common::CSwitchToConnectedEvent() );
}


common::CRequest< MonitorResponses >*
CAddTrackerAction::execute()
{
	return m_request;
}

std::vector< unsigned char >
CAddTrackerAction::getPayload() const
{
	return m_payload;
}

unsigned int
CAddTrackerAction::getMediumKind() const
{
	return m_mediumKind;
}

void
CAddTrackerAction::setMediumKind( unsigned int _mediumKind )
{
	m_mediumKind = _mediumKind;
}

void
CAddTrackerAction::accept( common::CSetResponseVisitor< MonitorResponses > & _visitor )
{
	_visitor.visit( *this );
}

void
CAddTrackerAction::setRequest( common::CRequest< MonitorResponses >* _request )
{
	m_request = _request;
}

}
