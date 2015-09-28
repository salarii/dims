// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/setResponseVisitor.h"
#include "common/responseVisitorInternal.h"
#include "common/events.h"

#include "monitor/connectNodeAction.h"
#include "monitor/updateDataAction.h"
#include "monitor/admitTrackerAction.h"
#include "monitor/admitTransactionsBundle.h"
#include "monitor/pingAction.h"
#include "monitor/recognizeNetworkAction.h"
#include "monitor/trackOriginAddressAction.h"
#include "monitor/provideInfoAction.h"
#include "monitor/synchronizationAction.h"
#include "monitor/enterNetworkAction.h"
#include "monitor/passTransactionAction.h"

namespace common
{

void
CSetResponseVisitor::visit( monitor::CConnectNodeAction & _action )
{
	boost::apply_visitor(
				( CResponseVisitorBase< monitor::CConnectNodeAction, common::DimsResponsesList > const & )
				CSetResult<monitor::CConnectNodeAction>( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( monitor::CUpdateDataAction & _action )
{
	boost::apply_visitor(
				( CResponseVisitorBase< monitor::CUpdateDataAction, common::DimsResponsesList > const & )
				CSetResult<monitor::CUpdateDataAction>( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( monitor::CAdmitTrackerAction & _action )
{
	boost::apply_visitor(
				( CResponseVisitorBase< monitor::CAdmitTrackerAction, common::DimsResponsesList > const & )
				CSetResult<monitor::CAdmitTrackerAction>( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( monitor::CAdmitTransactionBundle & _action )
{
	boost::apply_visitor(
				( CResponseVisitorBase< monitor::CAdmitTransactionBundle, common::DimsResponsesList > const & )
				CSetResult<monitor::CAdmitTransactionBundle>( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( monitor::CPingAction & _action )
{
	boost::apply_visitor(
				( CResponseVisitorBase< monitor::CPingAction, common::DimsResponsesList > const & )
				CSetResult<monitor::CPingAction>( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( monitor::CRecognizeNetworkAction & _action )
{
	boost::apply_visitor(
				( CResponseVisitorBase< monitor::CRecognizeNetworkAction, common::DimsResponsesList > const & )
				CSetResult<monitor::CRecognizeNetworkAction>( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( monitor::CTrackOriginAddressAction & _action )
{
	boost::apply_visitor(
				( CResponseVisitorBase< monitor::CTrackOriginAddressAction, common::DimsResponsesList > const & )
				CSetResult<monitor::CTrackOriginAddressAction>( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( monitor::CProvideInfoAction & _action )
{
	boost::apply_visitor(
				( CResponseVisitorBase< monitor::CProvideInfoAction, common::DimsResponsesList > const & )
				CSetResult<monitor::CProvideInfoAction>( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( monitor::CCopyTransactionStorageAction & _action )
{}

void
CSetResponseVisitor::visit( monitor::CSynchronizationAction & _action )
{
	boost::apply_visitor(
				( CResponseVisitorBase< monitor::CSynchronizationAction, common::DimsResponsesList > const & )
				CSetResult<monitor::CSynchronizationAction>( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( monitor::CEnterNetworkAction & _action )
{
	boost::apply_visitor(
				( CResponseVisitorBase< monitor::CEnterNetworkAction, common::DimsResponsesList > const & )
				CSetResult<monitor::CEnterNetworkAction>( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( monitor::CPassTransactionAction & _action )
{
	boost::apply_visitor(
				( CResponseVisitorBase< monitor::CPassTransactionAction, common::DimsResponsesList > const & )
				CSetResult<monitor::CPassTransactionAction>( &_action ), m_responses );
}

}

