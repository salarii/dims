// Copyright (c) 2014 Ratcoin dev-team
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

	virtual void operator()( tracker::CConnectedNode & _param ) const
	{
		this->m_action->process_event( common::CNodeConnectedEvent( _param.m_node ) );
	}

	virtual void operator()( common::CIdentificationResult & _param ) const
	{
		this->m_action->process_event( common::CIntroduceEvent( _param.m_address ) );
	}

	virtual void operator()( common::CContinueResult & _param ) const
	{
		this->m_action->process_event( common::CContinueEvent( _param.m_id ) );
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

