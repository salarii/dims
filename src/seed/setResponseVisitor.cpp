// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/setResponseVisitor.h"
#include "common/responseVisitorInternal.h"
#include "common/events.h"

#include "seed/acceptNodeAction.h"
#include "seed/pingAction.h"

namespace common
{

class CSetNodeConnectedResult : public CResponseVisitorBase< seed::CAcceptNodeAction, common::DimsResponsesList >
{
public:
	CSetNodeConnectedResult( seed::CAcceptNodeAction * const _action ):CResponseVisitorBase< seed::CAcceptNodeAction, common::DimsResponsesList >( _action ){};

	virtual void operator()( common::CConnectedNode & _param ) const
	{
		LogPrintf("set response \"connect node\" to action: %p \n", this->m_action );

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

	virtual void operator()( common::CTimeEvent & _param ) const
	{
		LogPrintf("set response \"time event\" to action: %p \n", this->m_action );

		this->m_action->process_event( _param );
	}

	virtual void operator()( common::CAckResult & _param ) const
	{
		LogPrintf("set response \"ack\" to action: %p \n", this->m_action );
		this->m_action->process_event( common::CAckEvent( _param.m_nodePtr ) );
	}

	virtual void operator()( common::CMessageResult & _param ) const
	{
		LogPrintf("set response \"message result\" to action: %p \n", this->m_action );
		this->m_action->process_event( _param );
	}
};

class CSetPingResult : public CResponseVisitorBase< seed::CPingAction, common::DimsResponsesList >
{
public:
	CSetPingResult( seed::CPingAction * const _action ):CResponseVisitorBase< seed::CPingAction, common::DimsResponsesList >( _action ){};

	virtual void operator()( common::CTimeEvent & _param ) const
	{
		this->m_action->process_event( _param );
	}

	virtual void operator()( common::CPingPongResult & _param ) const
	{
		this->m_action->process_event( _param );
	}
};

void
CSetResponseVisitor::visit( seed::CAcceptNodeAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< seed::CAcceptNodeAction, common::DimsResponsesList > const &)CSetNodeConnectedResult( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( seed::CPingAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< seed::CPingAction, common::DimsResponsesList > const &)CSetPingResult( &_action ), m_responses );
}


}

