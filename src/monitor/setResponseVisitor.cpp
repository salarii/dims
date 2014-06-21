// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/setResponseVisitor.h"
#include "common/responseVisitorInternal.h"

#include "addTrackerAction.h"

#include "common/commonEvents.h"

namespace common
{


template < class _Action >
class CSetNodeConnectedResult : public CResponseVisitorBase< _Action, monitor::MonitorResponseList >
{
public:
	CSetNodeConnectedResult( _Action * const _action ):CResponseVisitorBase< _Action, monitor::MonitorResponseList >( _action ){};

	virtual void operator()( common::CIdentificationResult & _param ) const
	{
		this->m_action->process_event( common::CIntroduceEvent(_param.m_address) );
	}

	virtual void operator()( common::CContinueResult & _param ) const
	{
		this->m_action->process_event( common::CContinueEvent( _param.m_id ) );
	}
};


CSetResponseVisitor< monitor::MonitorResponses >::CSetResponseVisitor( monitor::MonitorResponses const & _requestResponse )
	: m_requestResponse( _requestResponse )
{
}


void
CSetResponseVisitor< monitor::MonitorResponses >::visit( monitor::CAddTrackerAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< monitor::CAddTrackerAction, monitor::MonitorResponseList > const &)CSetNodeConnectedResult< monitor::CAddTrackerAction >( &_action ), m_requestResponse );
}

}
