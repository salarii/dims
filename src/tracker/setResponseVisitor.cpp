// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/setResponseVisitor.h"
#include "common/responseVisitorInternal.h"
#include "common/commonEvents.h"

#include "tracker/getBalanceAction.h"
#include "tracker/validateTransactionsAction.h"
#include "tracker/trackerEvents.h"
#include "tracker/connectNodeAction.h"
#include "tracker/trackOriginAddressAction.h"
#include "tracker/synchronizationAction.h"
#include "tracker/provideInfoAction.h"

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
		this->m_action->process_event( tracker::CValidationEvent( _param.m_invalidTransactionIndexes ) );
	}

	virtual void operator()( common::CMessageResult & _param ) const
	{
		this->m_action->process_event( _param );
	}

	virtual void operator()( common::CAckResult & _param ) const
	{
		this->m_action->process_event( common::CAckEvent( _param.m_nodePtr ) );
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

	virtual void operator()( common::CAckResult & _param ) const
	{
		this->m_action->process_event( common::CAckEvent() );
	}

	virtual void operator()( common::CGetPrompt & _param ) const
	{
		this->m_action->process_event( common::CGetEvent(_param.m_type) );
	}

	virtual void operator()( common::CMessageResult & _param ) const
	{
		this->m_action->process_event( _param );
	}
};

class CSetTrackOriginAddressResult : public CResponseVisitorBase< tracker::CTrackOriginAddressAction, tracker::TrackerResponseList >
{
public:
	CSetTrackOriginAddressResult( tracker::CTrackOriginAddressAction * const _action ):CResponseVisitorBase< tracker::CTrackOriginAddressAction, tracker::TrackerResponseList >( _action ){};

	virtual void operator()( tracker::CRequestedMerkles & _param ) const
	{
		this->m_action->process_event( tracker::CMerkleBlocksEvent( _param.m_merkles, _param.m_transactions, _param.m_id ) );
	}

	virtual void operator()( common::CTimeEvent & _param ) const
	{
		this->m_action->process_event( _param );
	}
};


class CSetSynchronizationResult : public CResponseVisitorBase< tracker::CSynchronizationAction, tracker::TrackerResponseList >
{
public:
	CSetSynchronizationResult( tracker::CSynchronizationAction * const _action ):CResponseVisitorBase< tracker::CSynchronizationAction, tracker::TrackerResponseList >( _action ){};

	virtual void operator()( tracker::CRequestedMerkles & _param ) const
	{
		this->m_action->process_event( tracker::CMerkleBlocksEvent( _param.m_merkles, _param.m_transactions, _param.m_id ) );
	}

	virtual void operator()( tracker::CSynchronizationInfoResult & _param ) const
	{
		this->m_action->process_event( tracker::CSynchronizationInfoEvent( _param.m_timeStamp, _param.m_nodeIndicator ) );
	}

	virtual void operator()( tracker::CSynchronizationBlockResult<tracker::CDiskBlock> & _param ) const
	{
		this->m_action->process_event( tracker::CTransactionBlockEvent<tracker::CDiskBlock>( _param.m_discBlock, _param.m_blockIndex ) );
	}

	virtual void operator()( tracker::CSynchronizationBlockResult<tracker::CSegmentHeader> & _param ) const
	{
		this->m_action->process_event( tracker::CTransactionBlockEvent<tracker::CSegmentHeader>( _param.m_discBlock, _param.m_blockIndex ) );
	}

	virtual void operator()( common::CGetPrompt & _param ) const
	{
		this->m_action->process_event( common::CGetEvent( _param.m_type ) );
	}

	virtual void operator()( common::CEndEvent & _param ) const
	{
		this->m_action->process_event( _param );
	}

	virtual void operator()( common::CAckResult & _param ) const
	{
		this->m_action->process_event( common::CAckEvent() );
	}
};

class CSetProvideInfoResult : public CResponseVisitorBase< tracker::CProvideInfoAction, tracker::TrackerResponseList >
{
public:
	CSetProvideInfoResult( tracker::CProvideInfoAction * const _action ):CResponseVisitorBase< tracker::CProvideInfoAction, tracker::TrackerResponseList >( _action ){};

	virtual void operator()( common::CMessageResult & _param ) const
	{
		this->m_action->process_event( _param );
	}

	virtual void operator()( common::CAckResult & _param ) const
	{
		this->m_action->process_event( common::CAckEvent() );
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
	boost::apply_visitor( (CResponseVisitorBase< tracker::CTrackOriginAddressAction, tracker::TrackerResponseList > const &)CSetTrackOriginAddressResult( &_action ), m_trackerResponses );
}

void
CSetResponseVisitor< tracker::TrackerResponses >::visit( tracker::CSynchronizationAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< tracker::CSynchronizationAction, tracker::TrackerResponseList > const &)CSetSynchronizationResult( &_action ), m_trackerResponses );
}

void
CSetResponseVisitor< tracker::TrackerResponses >::visit( tracker::CProvideInfoAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< tracker::CProvideInfoAction, tracker::TrackerResponseList > const &)CSetProvideInfoResult( &_action ), m_trackerResponses );
}

}
