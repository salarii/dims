// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/setResponseVisitor.h"
#include "common/responseVisitorInternal.h"
#include "common/commonEvents.h"

#include "tracker/getBalanceAction.h"
#include "tracker/validateTransactionsAction.h"
#include "tracker/events.h"
#include "tracker/connectNodeAction.h"
#include "tracker/trackOriginAddressAction.h"
#include "tracker/synchronizationAction.h"
#include "tracker/provideInfoAction.h"
#include "tracker/pingAction.h"
#include "tracker/registerAction.h"
#include "tracker/recognizeNetworkAction.h"
#include "tracker/passTransactionAction.h"
#include "tracker/connectNetworkAction.h"

namespace common
{

class GetBalance : public CResponseVisitorBase< tracker::CGetBalanceAction, tracker::TrackerResponseList >
{
public:
	GetBalance( tracker::CGetBalanceAction * const _action ):CResponseVisitorBase< tracker::CGetBalanceAction, tracker::TrackerResponseList >( _action ){};

	virtual void operator()( common::CAvailableCoinsData & _param ) const
	{
		LogPrintf("set response \"available coins\" to action: %p \n", this->m_action );
		this->m_action->process_event( _param );
	}

	virtual void operator()( common::CTimeEvent & _param ) const
	{
		LogPrintf("set response \"time event\" to action: %p \n", this->m_action );
		this->m_action->process_event( _param );
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
};

class CSetValidationResult : public CResponseVisitorBase< tracker::CValidateTransactionsAction, tracker::TrackerResponseList >
{
public:
	CSetValidationResult( tracker::CValidateTransactionsAction * const _action ):CResponseVisitorBase< tracker::CValidateTransactionsAction, tracker::TrackerResponseList >( _action ){};

	virtual void operator()( common::CValidationResult & _param ) const
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

	virtual void operator()( common::CTimeEvent & _param ) const
	{
		LogPrintf("set response \"time event\" to action: %p \n", this->m_action );
		this->m_action->process_event( _param );
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

class CSetTrackOriginAddressResult : public CResponseVisitorBase< tracker::CTrackOriginAddressAction, tracker::TrackerResponseList >
{
public:
	CSetTrackOriginAddressResult( tracker::CTrackOriginAddressAction * const _action ):CResponseVisitorBase< tracker::CTrackOriginAddressAction, tracker::TrackerResponseList >( _action ){};

	virtual void operator()( common::CRequestedMerkles & _param ) const
	{
		LogPrintf("set response \"requested merkles\" to action: %p \n", this->m_action );
		this->m_action->process_event( common::CMerkleBlocksEvent( _param.m_merkles, _param.m_transactions, _param.m_nodePtr ) );
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

	virtual void operator()( common::CTimeEvent & _param ) const
	{
		LogPrintf("set response \"time event\" to action: %p \n", this->m_action );
		this->m_action->process_event( _param );
	}
};

class CSetRecognizeNetworkResult : public CResponseVisitorBase< tracker::CRecognizeNetworkAction, tracker::TrackerResponseList >
{
public:
	CSetRecognizeNetworkResult( tracker::CRecognizeNetworkAction * const _action ):CResponseVisitorBase< tracker::CRecognizeNetworkAction, tracker::TrackerResponseList >( _action ){};

	virtual void operator()( common::ScheduledResult & _param ) const
	{
		LogPrintf("set response \"schedule result\" to action: %p \n", this->m_action );
		boost::apply_visitor( common::CResolveScheduledResult< tracker::CRecognizeNetworkAction >( this->m_action ), _param );
	}

	virtual void operator()( common::CTimeEvent & _param ) const
	{
		LogPrintf("set response \"time event\" to action: %p \n", this->m_action );
		this->m_action->process_event( _param );
	}
};

class CSetConnectNetworkResult : public CResponseVisitorBase< tracker::CConnectNetworkAction, tracker::TrackerResponseList >
{
public:
	CSetConnectNetworkResult( tracker::CConnectNetworkAction * const _action ):CResponseVisitorBase< tracker::CConnectNetworkAction, tracker::TrackerResponseList >( _action ){};

	virtual void operator()( common::ScheduledResult & _param ) const
	{
		LogPrintf("set response \"schedule result\" to action: %p \n", this->m_action );
		boost::apply_visitor( common::CResolveScheduledResult< tracker::CConnectNetworkAction >( this->m_action ), _param );
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

	virtual void operator()( common::CTimeEvent & _param ) const
	{
		LogPrintf("set response \"time event\" to action: %p \n", this->m_action );
		this->m_action->process_event( _param );
	}

	virtual void operator()( common::CAvailableCoinsData & _param ) const
	{
		LogPrintf("set response \"available coins\" to action: %p \n", this->m_action );
		this->m_action->process_event( _param );
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

class CSetPassTransactionResult : public CResponseVisitorBase< tracker::CPassTransactionAction, tracker::TrackerResponseList >
{
public:
	CSetPassTransactionResult( tracker::CPassTransactionAction * const _action ):CResponseVisitorBase< tracker::CPassTransactionAction, tracker::TrackerResponseList >( _action ){};

	virtual void operator()( common::CTimeEvent & _param ) const
	{
		LogPrintf("set response \"time event\" to action: %p \n", this->m_action );
		this->m_action->process_event( _param );
	}

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

	virtual void operator()( common::ScheduledResult & _param ) const
	{
		boost::apply_visitor( common::CResolveScheduledResult< tracker::CPassTransactionAction >( this->m_action ), _param );
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

	virtual void operator()( common::ScheduledResult & _param ) const
	{
		boost::apply_visitor( common::CResolveScheduledResult< tracker::CRegisterAction >( this->m_action ), _param );
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

void
CSetResponseVisitor< CTrackerTypes >::visit( tracker::CRecognizeNetworkAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< tracker::CRecognizeNetworkAction, tracker::TrackerResponseList > const &)CSetRecognizeNetworkResult( &_action ), m_trackerResponses );
}

void
CSetResponseVisitor< CTrackerTypes >::visit( tracker::CConnectNetworkAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< tracker::CConnectNetworkAction, tracker::TrackerResponseList > const &)CSetConnectNetworkResult( &_action ), m_trackerResponses );
}

void
CSetResponseVisitor< CTrackerTypes >::visit( tracker::CPassTransactionAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< tracker::CPassTransactionAction, tracker::TrackerResponseList > const &)CSetPassTransactionResult( &_action ), m_trackerResponses );
}

}
