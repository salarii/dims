// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/setResponseVisitor.h"
#include "common/responseVisitorInternal.h"
#include "getBalanceAction.h"

#include "validateTransactionActionEvents.h"

namespace common
{

template < class _Action >
class GetBalance : public CResponseVisitorBase< _Action, tracker::TrackerResponseList >
{
public:
	GetBalance( _Action * const _action ):CResponseVisitorBase< _Action, tracker::TrackerResponseList >( _action ){};

	virtual void operator()( common::CAvailableCoins & _param ) const
	{
		this->m_action->passBalance( _param );
	}
};

template < class _Action >
class CSetValidationResult : public CResponseVisitorBase< _Action, tracker::TrackerResponseList >
{
public:
	CSetValidationResult( _Action * const _action ):CResponseVisitorBase< _Action, tracker::TrackerResponseList >( _action ){};

	virtual void operator()( tracker::CValidationResult & _param ) const
	{
		this->m_action->process_event( tracker::CValidationEvent( _param.m_valid ) );
	}
};




CSetResponseVisitor< tracker::TrackerResponses >::CSetResponseVisitor( tracker::TrackerResponses const & _trackerResponse )
	: m_trackerResponses( _trackerResponse )
{
}

void
CSetResponseVisitor< tracker::TrackerResponses >::visit( common::CAction< tracker::TrackerResponses > & _action )
{

}


void
CSetResponseVisitor< tracker::TrackerResponses >::visit( tracker::CGetBalanceAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< tracker::CGetBalanceAction, tracker::TrackerResponseList > const &)GetBalance< tracker::CGetBalanceAction >( &_action ), m_trackerResponses );
}

}
