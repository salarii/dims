// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "core.h"

#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "common/authenticationProvider.h"
#include "common/setResponseVisitor.h"
#include "common/analyseTransaction.h"

#include "monitor/admitTrackerAction.h"
#include "monitor/monitorController.h"

namespace monitor
{

bool analyseTransaction( CTransaction & _transaction, uint256 const & _hash, CKeyID const & _trackerId )
{
	if ( !common::findKeyInInputs( _transaction, _trackerId ) )
		return false;

	CTxOut txOut;
	unsigned id;

	bool outputExist = common::findOutputInTransaction(
				_transaction
				, common::CAuthenticationProvider::getInstance()->getMyKey().GetID()
				, txOut
				, id );

	return txOut.nValue >= CMonitorController::getInstance()->getPrice();
}

struct CWaitForInfo : boost::statechart::state< CWaitForInfo, CAdmitTrackerAction >
{
	CWaitForInfo( my_context ctx ) : my_base( ctx )
	{
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CMessageResult >
	> reactions;
};

CAdmitTrackerAction::CAdmitTrackerAction( uint256 const & _actionKey, uintptr_t _mediumPtr )
{
	initiate();
}

void
CAdmitTrackerAction::accept( common::CSetResponseVisitor< common::CMonitorTypes > & _visitor )
{
	_visitor.visit( *this );
}

}
