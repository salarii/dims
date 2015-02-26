// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/setResponseVisitor.h"
#include "common/responseVisitorInternal.h"
#include "common/commonEvents.h"

#include "acceptNodeAction.h"

namespace common
{

template < class _Action >
class CSetNodeConnectedResult : public CResponseVisitorBase< _Action, seed::SeedResponseList >
{
public:
	CSetNodeConnectedResult( _Action * const _action ):CResponseVisitorBase< _Action, seed::SeedResponseList >( _action ){};

	virtual void operator()( common::CConnectedNode & _param ) const
	{
		if ( _param.m_node )
			this->m_action->process_event( common::CNodeConnectedEvent( _param.m_node ) );
		else
			this->m_action->process_event( common::CCantReachNode() );
	}

	virtual void operator()( common::CIdentificationResult & _param ) const
	{
		this->m_action->process_event( _param );
	}

	virtual void operator()( common::CRoleResult & _param ) const
	{
		this->m_action->process_event( common::CRoleEvent( _param.m_role ) );
	}

	virtual void operator()( common::CNetworkInfoResult & _param ) const
	{
		this->m_action->process_event( common::CNetworkInfoEvent( _param.m_trackersInfo, _param.m_monitorsInfo ) );
	}

	virtual void operator()( common::CTimeEvent & _param ) const
	{
		this->m_action->process_event( _param );
	}

	virtual void operator()( common::CAckResult & _param ) const
	{
		this->m_action->process_event( common::CAckEvent( _param.m_nodePtr ) );
	}
};


CSetResponseVisitor< seed::SeedResponses >::CSetResponseVisitor( seed::SeedResponses const & _requestResponse )
	: m_requestResponse( _requestResponse )
{
}


void
CSetResponseVisitor< seed::SeedResponses >::visit( seed::CAcceptNodeAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< seed::CAcceptNodeAction, seed::SeedResponseList > const &)CSetNodeConnectedResult< seed::CAcceptNodeAction >( &_action ), m_requestResponse );
}

}

