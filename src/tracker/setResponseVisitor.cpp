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
#include "tracker/pingAction.h"
#include "tracker/registerAction.h"

namespace common
{

class GetBalance : public CResponseVisitorBase< tracker::CGetBalanceAction, tracker::TrackerResponseList >
{
public:
	GetBalance( tracker::CGetBalanceAction * const _action ):CResponseVisitorBase< tracker::CGetBalanceAction, tracker::TrackerResponseList >( _action ){};

	virtual void operator()( common::CAvailableCoins & _param ) const
	{
		LogPrintf("set response \"available coins\" to action: %p \n", this->m_action );
		this->m_action->process_event( _param );
	}
};

class CSetValidationResult : public CResponseVisitorBase< tracker::CValidateTransactionsAction, tracker::TrackerResponseList >
{
public:
	CSetValidationResult( tracker::CValidateTransactionsAction * const _action ):CResponseVisitorBase< tracker::CValidateTransactionsAction, tracker::TrackerResponseList >( _action ){};

	virtual void operator()( tracker::CValidationResult & _param ) const
	{
		LogPrintf("set response \"validation result\" to action: %p \n", this->m_action );
		this->m_action->process_event( tracker::CValidationEvent( _param.m_invalidTransactionIndexes ) );
	}

	virtual void operator()( common::CMessageResult & _param ) const
	{
		LogPrintf("set response \"message result\" to action: %p \n", this->m_action );
		this->m_action->process_event( _param );
	}

	virtual void operator()( common::CAckResult & _param ) const
	{
		LogPrintf("set response \"ack\" to action: %p \n", this->m_action );
		this->m_action->process_event( common::CAckEvent( _param.m_nodePtr ) );
	}

	virtual void operator()( common::CErrorEvent & _param ) const
	{
		LogPrintf("set response \"error\" to action: %p \n", this->m_action );
		//handle it somehow
	}
};

class CSetNodeConnectedResult : public CResponseVisitorBase< tracker::CConnectNodeAction, tracker::TrackerResponseList >
{
public:
	CSetNodeConnectedResult( tracker::CConnectNodeAction * const _action ):CResponseVisitorBase< tracker::CConnectNodeAction, tracker::TrackerResponseList >( _action ){};

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

	virtual void operator()( common::CRoleResult & _param ) const
	{
		LogPrintf("set response \"role\" to action: %p \n", this->m_action );
		this->m_action->process_event( common::CRoleEvent( _param.m_role ) );
	}

	virtual void operator()( common::CNetworkInfoResult & _param ) const
	{
		LogPrintf("set response \"network info\" to action: %p \n", this->m_action );
		this->m_action->process_event( common::CNetworkInfoEvent( _param.m_trackersInfo, _param.m_monitorsInfo ) );
	}

	virtual void operator()( common::CAckResult & _param ) const
	{
		LogPrintf("set response \"ack\" to action: %p \n", this->m_action );
		this->m_action->process_event( common::CAckEvent() );
	}

	virtual void operator()( common::CGetPrompt & _param ) const
	{
		LogPrintf("set response \"get prompt\" to action: %p \n", this->m_action );
		this->m_action->process_event( common::CGetEvent(_param.m_type) );
	}

	virtual void operator()( common::CMessageResult & _param ) const
	{
		LogPrintf("set response \"message result\" to action: %p \n", this->m_action );
		this->m_action->process_event( _param );
	}
};

class CSetTrackOriginAddressResult : public CResponseVisitorBase< tracker::CTrackOriginAddressAction, tracker::TrackerResponseList >
{
public:
	CSetTrackOriginAddressResult( tracker::CTrackOriginAddressAction * const _action ):CResponseVisitorBase< tracker::CTrackOriginAddressAction, tracker::TrackerResponseList >( _action ){};

	virtual void operator()( tracker::CRequestedMerkles & _param ) const
	{
		LogPrintf("set response \"requested merkles\" to action: %p \n", this->m_action );
		this->m_action->process_event( tracker::CMerkleBlocksEvent( _param.m_merkles, _param.m_transactions, _param.m_id ) );
	}

	virtual void operator()( common::CTimeEvent & _param ) const
	{
		LogPrintf("set response \"time event\" to action: %p \n", this->m_action );
		this->m_action->process_event( _param );
	}
};

class CSetSynchronizationResult : public CResponseVisitorBase< tracker::CSynchronizationAction, tracker::TrackerResponseList >
{
public:
	CSetSynchronizationResult( tracker::CSynchronizationAction * const _action ):CResponseVisitorBase< tracker::CSynchronizationAction, tracker::TrackerResponseList >( _action ){};

	virtual void operator()( tracker::CRequestedMerkles & _param ) const
	{
		LogPrintf("set response \"requested merkles\" to action: %p \n", this->m_action );
		this->m_action->process_event( tracker::CMerkleBlocksEvent( _param.m_merkles, _param.m_transactions, _param.m_id ) );
	}

	virtual void operator()( tracker::CSynchronizationInfoResult & _param ) const
	{
		LogPrintf("set response \"synchronization result\" to action: %p \n", this->m_action );
		this->m_action->process_event( tracker::CSynchronizationInfoEvent( _param.m_timeStamp, _param.m_nodeIndicator ) );
	}

	virtual void operator()( tracker::CSynchronizationBlockResult<tracker::CDiskBlock> & _param ) const
	{
		LogPrintf("set response \"synchronization block, disc block\" to action: %p \n", this->m_action );
		this->m_action->process_event( tracker::CTransactionBlockEvent<tracker::CDiskBlock>( _param.m_discBlock, _param.m_blockIndex ) );
	}

	virtual void operator()( tracker::CSynchronizationBlockResult<tracker::CSegmentHeader> & _param ) const
	{
		LogPrintf("set response \"synchronization block, segment header\" to action: %p \n", this->m_action );
		this->m_action->process_event( tracker::CTransactionBlockEvent<tracker::CSegmentHeader>( _param.m_discBlock, _param.m_blockIndex ) );
	}

	virtual void operator()( common::CGetPrompt & _param ) const
	{
		LogPrintf("set response \"get prompt\" to action: %p \n", this->m_action );
		this->m_action->process_event( common::CGetEvent( _param.m_type ) );
	}

	virtual void operator()( common::CEndEvent & _param ) const
	{
		LogPrintf("set response \"end event\" to action: %p \n", this->m_action );
		this->m_action->process_event( _param );
	}

	virtual void operator()( common::CAckResult & _param ) const
	{
		LogPrintf("set response \"ack\" to action: %p \n", this->m_action );
		this->m_action->process_event( common::CAckEvent() );
	}
};

class CSetProvideInfoResult : public CResponseVisitorBase< tracker::CProvideInfoAction, tracker::TrackerResponseList >
{
public:
	CSetProvideInfoResult( tracker::CProvideInfoAction * const _action ):CResponseVisitorBase< tracker::CProvideInfoAction, tracker::TrackerResponseList >( _action ){};

	virtual void operator()( common::CMessageResult & _param ) const
	{
		LogPrintf("set response \"message result\" to action: %p \n", this->m_action );
		this->m_action->process_event( _param );
	}

	virtual void operator()( common::CAckResult & _param ) const
	{
		LogPrintf("set response \"ack\" to action: %p \n", this->m_action );
		this->m_action->process_event( common::CAckEvent() );
	}
};

class CSetPingResult : public CResponseVisitorBase< tracker::CPingAction, tracker::TrackerResponseList >
{
public:
	CSetPingResult( tracker::CPingAction * const _action ):CResponseVisitorBase< tracker::CPingAction, tracker::TrackerResponseList >( _action ){};

	virtual void operator()( common::CTimeEvent & _param ) const
	{
		LogPrintf("set response \"time event\" to action: %p \n", this->m_action );
		this->m_action->process_event( _param );
	}

	virtual void operator()( common::CPingPongResult & _param ) const
	{
		LogPrintf("set response \"ping pong\" to action: %p \n", this->m_action );
		this->m_action->process_event( _param );
	}
};

class CSetRegisterAction : public CResponseVisitorBase< tracker::CRegisterAction, tracker::TrackerResponseList >
{
public:
	CSetRegisterAction( tracker::CRegisterAction * const _action ):CResponseVisitorBase< tracker::CRegisterAction, tracker::TrackerResponseList >( _action ){};

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

	virtual void operator()( common::CTimeEvent & _param ) const
	{
		LogPrintf("set response \"time event\" to action: %p \n", this->m_action );
		this->m_action->process_event( _param );
	}
};


CSetResponseVisitor< CTrackerTypes >::CSetResponseVisitor( tracker::TrackerResponses const & _trackerResponse )
	: m_trackerResponses( _trackerResponse )
{
}

void
CSetResponseVisitor< CTrackerTypes >::visit( common::CAction< CTrackerTypes > & _action )
{
}

void
CSetResponseVisitor< CTrackerTypes >::visit( tracker::CGetBalanceAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< tracker::CGetBalanceAction, tracker::TrackerResponseList > const &)GetBalance( &_action ), m_trackerResponses );
}

void
CSetResponseVisitor< CTrackerTypes >::visit( tracker::CValidateTransactionsAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< tracker::CValidateTransactionsAction, tracker::TrackerResponseList > const &)CSetValidationResult( &_action ), m_trackerResponses );
}

void
CSetResponseVisitor< CTrackerTypes >::visit( tracker::CConnectNodeAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< tracker::CConnectNodeAction, tracker::TrackerResponseList > const &)CSetNodeConnectedResult( &_action ), m_trackerResponses );
}

void
CSetResponseVisitor< CTrackerTypes >::visit( tracker::CTrackOriginAddressAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< tracker::CTrackOriginAddressAction, tracker::TrackerResponseList > const &)CSetTrackOriginAddressResult( &_action ), m_trackerResponses );
}

void
CSetResponseVisitor< CTrackerTypes >::visit( tracker::CSynchronizationAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< tracker::CSynchronizationAction, tracker::TrackerResponseList > const &)CSetSynchronizationResult( &_action ), m_trackerResponses );
}

void
CSetResponseVisitor< CTrackerTypes >::visit( tracker::CProvideInfoAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< tracker::CProvideInfoAction, tracker::TrackerResponseList > const &)CSetProvideInfoResult( &_action ), m_trackerResponses );
}

void
CSetResponseVisitor< CTrackerTypes >::visit( tracker::CPingAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< tracker::CPingAction, tracker::TrackerResponseList > const &)CSetPingResult( &_action ), m_trackerResponses );
}

void
CSetResponseVisitor< CTrackerTypes >::visit( tracker::CRegisterAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< tracker::CRegisterAction, tracker::TrackerResponseList > const &)CSetRegisterAction( &_action ), m_trackerResponses );
}

}
