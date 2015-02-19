// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/setResponseVisitor.h"
#include "common/responseVisitorInternal.h"
#include "common/commonEvents.h"
#include "monitor/connectNodeAction.h"
#include "monitor/updateDataAction.h"

namespace common
{

template < class _Action >
class CSetNodeConnectedResult : public CResponseVisitorBase< _Action, monitor::MonitorResponseList >
{
public:
	CSetNodeConnectedResult( _Action * const _action ):CResponseVisitorBase< _Action, monitor::MonitorResponseList >( _action ){};

	virtual void operator()( common::CConnectedNode & _param ) const
	{
		if ( _param.m_node )
			this->m_action->process_event( common::CNodeConnectedEvent( _param.m_node ) );
		else
			this->m_action->process_event( common::CCantReachNode() );
	}

	virtual void operator()( common::CIdentificationResult & _param ) const
	{
		this->m_action->process_event( common::CIntroduceEvent( _param.m_payload, _param.m_signed, _param.m_key, _param.m_address) );
	}

	virtual void operator()( common::CContinueResult & _param ) const
	{
		this->m_action->process_event( common::CContinueEvent( _param.m_id ) );
	}

	virtual void operator()( common::CNetworkInfoResult & _param ) const
	{
		this->m_action->process_event( common::CNetworkInfoEvent( _param.m_trackersInfo, _param.m_monitorsInfo ) );
	}

	virtual void operator()( common::CAckPromptResult & _param ) const
	{
		this->m_action->process_event( _param );
	}

	virtual void operator()( common::CAckResult & _param ) const
	{
		this->m_action->process_event( common::CAckEvent() );
	}

	virtual void operator()( common::CMessageResult & _param ) const
	{
		this->m_action->process_event( _param );
	}

	virtual void operator()( common::CGetPrompt & _param ) const
	{
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
		this->m_action->process_event( _param );
	}

	virtual void operator()( common::CNoMedium & _param ) const
	{
		this->m_action->process_event( _param );
	}

};


CSetResponseVisitor< monitor::MonitorResponses >::CSetResponseVisitor( monitor::MonitorResponses const & _requestResponse )
	: m_requestResponse( _requestResponse )
{
}

void
CSetResponseVisitor< monitor::MonitorResponses >::visit( monitor::CConnectNodeAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< monitor::CConnectNodeAction, monitor::MonitorResponseList > const &)CSetNodeConnectedResult< monitor::CConnectNodeAction >( &_action ), m_requestResponse );
}


void
CSetResponseVisitor< monitor::MonitorResponses >::visit( monitor::CUpdateDataAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< monitor::CUpdateDataAction, monitor::MonitorResponseList > const &)CSetUpdateDataResult< monitor::CUpdateDataAction >( &_action ), m_requestResponse );
}

}

