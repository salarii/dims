// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/setResponseVisitor.h"
#include "common/responseVisitorInternal.h"
#include "common/commonEvents.h"
#include "getBalanceAction.h"
#include "validateTransactionsAction.h"
#include "trackerEvents.h"
#include "connectNodeAction.h"
#include "trackOriginAddressAction.h"

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

	virtual void operator()( common::CErrorEvent & _param ) const
	{
		//handle it somehow
	}
};

template < class _Action >
class CSetNodeConnectedResult : public CResponseVisitorBase< _Action, tracker::TrackerResponseList >
{
public:
	CSetNodeConnectedResult( _Action * const _action ):CResponseVisitorBase< _Action, tracker::TrackerResponseList >( _action ){};

	virtual void operator()( common::CConnectedNode & _param ) const
	{
		this->m_action->process_event( common::CNodeConnectedEvent( _param.m_node ) );
	}

	virtual void operator()( common::CIdentificationResult & _param ) const
	{
		this->m_action->process_event( common::CIntroduceEvent( _param.m_payload, _param.m_signed, _param.m_key, _param.m_address) );
	}

	virtual void operator()( common::CContinueResult & _param ) const
	{
		this->m_action->process_event( common::CContinueEvent( _param.m_id ) );
	}

	virtual void operator()( common::CRoleResult & _param ) const
	{
		this->m_action->process_event( common::CRoleEvent( _param.m_role ) );
	}

	virtual void operator()( common::CNetworkInfoResult & _param ) const
	{
		this->m_action->process_event( common::CNetworkInfoEvent( _param.m_networkInfo ) );
	}

};

template < class _Action >
class CSynchronizationResult : public CResponseVisitorBase< _Action, tracker::TrackerResponseList >
{
public:
	CSynchronizationResult( _Action * const _action ):CResponseVisitorBase< _Action, tracker::TrackerResponseList >( _action ){};

	virtual void operator()( common::CContinueResult & _param ) const
	{
		this->m_action->process_event( common::CContinueEvent( _param.m_id ) );
	}

	virtual void operator()( tracker::CRequestedMerkles & _param ) const
	{
		this->m_action->process_event( tracker::CMerkleBlocksEvent( _param.m_merkles, _param.m_transactions, _param.m_id ) );
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

void
CSetResponseVisitor< tracker::TrackerResponses >::visit( tracker::CValidateTransactionsAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< tracker::CValidateTransactionsAction, tracker::TrackerResponseList > const &)CSetValidationResult< tracker::CValidateTransactionsAction >( &_action ), m_trackerResponses );
}

void
CSetResponseVisitor< tracker::TrackerResponses >::visit( tracker::CConnectNodeAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< tracker::CConnectNodeAction, tracker::TrackerResponseList > const &)CSetNodeConnectedResult< tracker::CConnectNodeAction >( &_action ), m_trackerResponses );
}

void
CSetResponseVisitor< tracker::TrackerResponses >::visit( tracker::CTrackOriginAddressAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< tracker::CTrackOriginAddressAction, tracker::TrackerResponseList > const &)CSynchronizationResult< tracker::CTrackOriginAddressAction >( &_action ), m_trackerResponses );
}


}
