// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/setResponseVisitor.h"
#include "common/responseVisitorInternal.h"
#include "common/commonEvents.h"

#include "monitor/connectNodeAction.h"
#include "monitor/updateDataAction.h"
#include "monitor/admitTrackerAction.h"
#include "monitor/admitTransactionsBundle.h"
#include "monitor/pingAction.h"

namespace common
{

template < class _Action >
class CSetNodeConnectedResult : public CResponseVisitorBase< _Action, monitor::MonitorResponseList >
{
public:
	CSetNodeConnectedResult( _Action * const _action ):CResponseVisitorBase< _Action, monitor::MonitorResponseList >( _action ){};

	virtual void operator()( common::CConnectedNode & _param ) const
	{
		LogPrintf("set response \"connected node\" to action: %p \n", this->m_action );

		if ( _param.m_node )
			this->m_action->process_event( common::CNodeConnectedEvent( _param.m_node ) );
		else
			this->m_action->process_event( common::CCantReachNode() );
	}

	virtual void operator()( common::CIdentificationResult & _param ) const
	{
		LogPrintf("set response \"identification result\" to action: %p \n", this->m_action );
		this->m_action->process_event( _param );
	}

	virtual void operator()( common::CNetworkInfoResult & _param ) const
	{
		LogPrintf("set response \"network info result\" to action: %p \n", this->m_action );
		this->m_action->process_event( common::CNetworkInfoEvent( _param.m_trackersInfo, _param.m_monitorsInfo ) );
	}

	virtual void operator()( common::CTimeEvent & _param ) const
	{
		LogPrintf("set response \"time event\" to action: %p \n", this->m_action );
		this->m_action->process_event( _param );
	}

	virtual void operator()( common::CAckResult & _param ) const
	{
		LogPrintf("set response \"ack\" to action: %p \n", this->m_action );
		this->m_action->process_event( common::CAckEvent() );
	}

	virtual void operator()( common::CMessageResult & _param ) const
	{
		LogPrintf("set response \"message result\" to action: %p \n", this->m_action );
		this->m_action->process_event( _param );
	}

	virtual void operator()( common::CGetPrompt & _param ) const
	{
		LogPrintf("set response \"get prompt\" to action: %p \n", this->m_action );
		this->m_action->process_event( common::CGetEvent(_param.m_type) );
	}
};

template < class _Action >
class CSetUpdateDataResult : public CResponseVisitorBase< _Action, monitor::MonitorResponseList >
{
public:
	CSetUpdateDataResult( _Action * const _action ):CResponseVisitorBase< _Action, monitor::MonitorResponseList >( _action ){};

	virtual void operator()( common::CMessageResult & _param ) const
	{
		LogPrintf("set response \"message result\" to action: %p \n", this->m_action );
		this->m_action->process_event( _param );
	}

	virtual void operator()( common::CNoMedium & _param ) const
	{
		LogPrintf("set response \"no medium\" to action: %p \n", this->m_action );
		this->m_action->process_event( _param );
	}
};


template < class _Action >
class CSetPingResult : public CResponseVisitorBase< _Action, monitor::MonitorResponseList >
{
public:
	CSetPingResult( _Action * const _action ):CResponseVisitorBase< _Action, monitor::MonitorResponseList >( _action ){};

	virtual void operator()( common::CTimeEvent & _param ) const
	{
		this->m_action->process_event( _param );
	}

	virtual void operator()( common::CPingPongResult & _param ) const
	{
		this->m_action->process_event( _param );
	}
};

CSetResponseVisitor< common::CMonitorTypes >::CSetResponseVisitor( monitor::MonitorResponses const & _requestResponse )
	: m_requestResponse( _requestResponse )
{
}

void
CSetResponseVisitor< common::CMonitorTypes >::visit( monitor::CConnectNodeAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< monitor::CConnectNodeAction, monitor::MonitorResponseList > const &)CSetNodeConnectedResult< monitor::CConnectNodeAction >( &_action ), m_requestResponse );
}

void
CSetResponseVisitor< common::CMonitorTypes >::visit( monitor::CUpdateDataAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< monitor::CUpdateDataAction, monitor::MonitorResponseList > const &)CSetUpdateDataResult< monitor::CUpdateDataAction >( &_action ), m_requestResponse );
}

void
CSetResponseVisitor< common::CMonitorTypes >::visit( monitor::CAdmitTrackerAction & _action )
{
		boost::apply_visitor( (CResponseVisitorBase< monitor::CAdmitTrackerAction, monitor::MonitorResponseList > const &)CSetUpdateDataResult< monitor::CAdmitTrackerAction >( &_action ), m_requestResponse );
}

void
CSetResponseVisitor< common::CMonitorTypes >::visit( monitor::CAdmitProofTransactionBundle & _action )
{
		boost::apply_visitor( (CResponseVisitorBase< monitor::CAdmitProofTransactionBundle, monitor::MonitorResponseList > const &)CSetUpdateDataResult< monitor::CAdmitProofTransactionBundle >( &_action ), m_requestResponse );
}

void
CSetResponseVisitor< common::CMonitorTypes >::visit( monitor::CPingAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< monitor::CPingAction, monitor::MonitorResponseList > const &)CSetPingResult< monitor::CPingAction >( &_action ), m_requestResponse );
}


}

