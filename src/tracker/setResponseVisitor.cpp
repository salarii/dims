// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/setResponseVisitor.h"
#include "common/responseVisitorInternal.h"
#include "common/events.h"

#include "tracker/getBalanceAction.h"
#include "tracker/validateTransactionsAction.h"
#include "tracker/events.h"
#include "tracker/connectNodeAction.h"
#include "tracker/trackOriginAddressAction.h"
#include "tracker/synchronizationAction.h"
#include "tracker/provideInfoAction.h"
#include "tracker/pingAction.h"
#include "tracker/registerAction.h"
#include "tracker/recognizeNetworkAction.h"
#include "tracker/passTransactionAction.h"
#include "tracker/connectNetworkAction.h"
#include "tracker/activityControllerAction.h"

namespace common
{

void
CSetResponseVisitor::visit( tracker::CGetBalanceAction & _action )
{
	boost::apply_visitor( CSetResult< tracker::CGetBalanceAction >( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( tracker::CValidateTransactionsAction & _action )
{
	boost::apply_visitor( CSetResult< tracker::CValidateTransactionsAction >( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( tracker::CConnectNodeAction & _action )
{
	boost::apply_visitor(CSetResult< tracker::CConnectNodeAction >( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( tracker::CTrackOriginAddressAction & _action )
{
	boost::apply_visitor( CSetResult< tracker::CTrackOriginAddressAction >( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( tracker::CSynchronizationAction & _action )
{
	boost::apply_visitor( CSetResult< tracker::CSynchronizationAction >( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( tracker::CProvideInfoAction & _action )
{
	boost::apply_visitor( CSetResult< tracker::CProvideInfoAction >( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( tracker::CPingAction & _action )
{
	boost::apply_visitor( CSetResult< tracker::CPingAction >( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( tracker::CRegisterAction & _action )
{
	boost::apply_visitor( CSetResult< tracker::CRegisterAction >( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( tracker::CRecognizeNetworkAction & _action )
{
	boost::apply_visitor( CSetResult< tracker::CRecognizeNetworkAction >( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( tracker::CConnectNetworkAction & _action )
{
	boost::apply_visitor( CSetResult< tracker::CConnectNetworkAction >( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( tracker::CPassTransactionAction & _action )
{
	boost::apply_visitor( CSetResult< tracker::CPassTransactionAction >( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( tracker::CActivityControllerAction & _action )
{
	boost::apply_visitor( CSetResult<tracker::CActivityControllerAction>( &_action ), m_responses );
}

}
