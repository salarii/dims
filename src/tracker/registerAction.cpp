// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "registerAction.h"

#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

namespace tracker
{

struct CInitiateRegistration : boost::statechart::state< CInitiateRegistration, CRegisterAction >
{
	CInitiateRegistration( my_context ctx )
		: my_base( ctx )
	{
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >
	> reactions;
};

CRegisterAction::CRegisterAction( uint256 const & _actionKey, uintptr_t _mediumPtr )
	: CCommunicationAction( _actionKey )
	, m_medium( _mediumPtr )
{
}

void
CRegisterAction::accept( common::CSetResponseVisitor< common::CTrackerTypes > & _visitor )
{
}

}
