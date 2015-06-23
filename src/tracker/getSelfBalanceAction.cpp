// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/statechart/state.hpp>

#include "common/setResponseVisitor.h"

#include "tracker/getSelfBalanceAction.h"

namespace tracker
{

struct CInitialState : boost::statechart::state< CInitialState, CGetSelfBalanceAction >
{
	CInitialState( my_context ctx ) : my_base( ctx )
	{}
};

struct CGetProveMessage : boost::statechart::state< CGetProveMessage, CGetSelfBalanceAction >
{
	CGetProveMessage( my_context ctx ) : my_base( ctx )
	{}
};

struct CRetriveBalance : boost::statechart::state< CRetriveBalance, CGetSelfBalanceAction >
{
	CRetriveBalance( my_context ctx ) : my_base( ctx )
	{}
};

struct CBalanceFromInternal : boost::statechart::state< CBalanceFromInternal, CGetSelfBalanceAction >
{
	CBalanceFromInternal( my_context ctx ) : my_base( ctx )
	{}
};

CGetSelfBalanceAction::CGetSelfBalanceAction()
{
	initiate();
}

void
CGetSelfBalanceAction::accept( common::CSetResponseVisitor< common::CTrackerTypes > & _visitor )
{
	_visitor.visit( *this );
}

}
