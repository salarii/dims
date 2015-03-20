#ifndef PING_ACTION_H
#define PING_ACTION_H

#include "common/action.h"
#include "common/filters.h"

#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

namespace common
{

template< class _Type >
class CPingAction;


template< class _Type >
struct CTryToReconnect : boost::statechart::state< CTryToReconnect< _Type >, CPingAction< _Type > >
{
	// do  cleanup
	typedef boost::mpl::list<
	> reactions;
};

template< class _Type >
struct CSendPing : boost::statechart::state< CSendPing< _Type >, CPingAction< _Type > >
{
	CSendPing( my_context ctx ) : my_base( ctx )
	{
		context< CSendPing >().dropRequests();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		CPingRequest
		CPongRequest
		context< CConnectNodeAction >().addRequests(
					new common::CKnownNetworkInfoRequest< common::CTrackerTypes >( context< CConnectNodeAction >().getActionKey()
																				   , knownNetworkInfo, new CSpecificMediumFilter( context< CConnectNodeAction >().getMediumPtr() ) ) );
		return discard_event();
	}

	boost::statechart::result react( common::CPingPongResult const & _pingPong )
	{
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CPingPongResult >
	> reactions;

};

template< class _Type >
class CPingAction : public common::CAction< _Type >, public  boost::statechart::state_machine< CPingAction, CSendPing< _Type > >, public common::CCommunicationAction
{
public:
	CPingAction( uint256 const & _actionKey );

	virtual void accept( common::CSetResponseVisitor< common::CTrackerTypes > & _visitor );

	uintptr_t getNodeIndicator()const;

	~CPingAction(){};
private:
	uintptr_t m_nodeIndicator;
};


}

#endif // PING_ACTION_H
