// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>


#include "common/setResponseVisitor.h"

#include "monitor/admitTransactionsBundle.h"

namespace monitor
{

CAdmitTransactionBundle * CAdmitTransactionBundle::ms_instance = NULL;

CAdmitTransactionBundle*
CAdmitTransactionBundle::getInstance()
{
	if ( !ms_instance )
	{
		ms_instance = new CAdmitTransactionBundle();
	};
	return ms_instance;
}

struct CWaitForBundle : boost::statechart::state< CWaitForBundle, CAdmitTransactionBundle >
{
	CWaitForBundle( my_context ctx ) : my_base( ctx )
	{
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CMessageResult >
	> reactions;
};

CAdmitTransactionBundle::CAdmitTransactionBundle()
	: common::CAction< common::CMonitorTypes >( false )
{
	initiate();
}

void
CAdmitTransactionBundle::accept( common::CSetResponseVisitor< common::CMonitorTypes > & _visitor )
{
	_visitor.visit( *this );
}

}
