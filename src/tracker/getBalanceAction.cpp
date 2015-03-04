// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "getBalanceAction.h"
#include "common/setResponseVisitor.h"
#include "clientRequestsManager.h"

#include "tracker/trackerRequests.h"

#include <boost/statechart/state.hpp>
#include <boost/statechart/custom_reaction.hpp>

namespace tracker
{

struct CFindBalance : boost::statechart::state< CFindBalance, CGetBalanceAction >
{
	CFindBalance( my_context ctx ) : my_base( ctx )
	{
		context< CGetBalanceAction >().dropRequests();

		context< CGetBalanceAction >().addRequests(
					new CGetBalanceRequest( context< CGetBalanceAction >().getKeyId() ) );
	}

	boost::statechart::result react( common::CAvailableCoins const & _availableCoins )
	{
		CClientRequestsManager::getInstance()->setClientResponse(
					  context< CGetBalanceAction >().getHash()
					, _availableCoins );

		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CAvailableCoins >
	> reactions;

};

CGetBalanceAction::CGetBalanceAction( uint160 const & _keyId, uint256 const & _hash )
	: m_keyId( _keyId )
	, m_hash( _hash )
{
	initiate();
}

void
CGetBalanceAction::accept( common::CSetResponseVisitor< TrackerResponses > & _visitor )
{
	_visitor.visit( *this );
}


}
