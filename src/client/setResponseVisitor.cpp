// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/setResponseVisitor.h"
#include "common/responses.h"
#include "common/events.h"
#include "common/responseVisitorInternal.h"

#include "sendTransactionAction.h"
#include "connectAction.h"
#include "sendBalanceInfoAction.h"
#include "payLocalApplicationAction.h"

#include "clientResponses.h"
#include "clientEvents.h"

#include <boost/any.hpp>
#include <boost/optional.hpp>

namespace common
{

void 
CSetResponseVisitor::visit( client::CSendTransactionAction & _action )
{
	boost::apply_visitor( CSetResult< client::CSendTransactionAction >( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( client::CConnectAction & _action )
{
	boost::apply_visitor( CSetResult< client::CConnectAction >( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( client::CSendBalanceInfoAction & _action )
{
	boost::apply_visitor( CSetResult< client::CSendBalanceInfoAction >( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( client::CPayLocalApplicationAction & _action )
{
	boost::apply_visitor( CSetResult< client::CPayLocalApplicationAction >( &_action ), m_responses );
}

}
