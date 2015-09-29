// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/setResponseVisitor.h"
#include "common/responseVisitorInternal.h"
#include "common/events.h"

#include "seed/acceptNodeAction.h"
#include "seed/pingAction.h"

namespace common
{

void
CSetResponseVisitor::visit( seed::CAcceptNodeAction & _action )
{
	boost::apply_visitor( CSetResult< seed::CAcceptNodeAction >( &_action ), m_responses  );
}

void
CSetResponseVisitor::visit( seed::CPingAction & _action )
{
	boost::apply_visitor( CSetResult< seed::CPingAction >( &_action ), m_responses  );
}


}

