// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/setResponseVisitor.h"
#include "common/responseVisitorInternal.h"
#include "common/events.h"

#include "monitor/connectNodeAction.h"
#include "monitor/updateDataAction.h"
#include "monitor/admitTrackerAction.h"
#include "monitor/admitTransactionsBundle.h"
#include "monitor/pingAction.h"
#include "monitor/recognizeNetworkAction.h"
#include "monitor/trackOriginAddressAction.h"
#include "monitor/provideInfoAction.h"
#include "monitor/synchronizationAction.h"
#include "monitor/enterNetworkAction.h"

namespace common
{

class CSetNodeConnectedResult : public CResponseVisitorBase< monitor::CConnectNodeAction, common::DimsResponsesList >
{
public:
	CSetNodeConnectedResult( monitor::CConnectNodeAction * const _action ):CResponseVisitorBase< monitor::CConnectNodeAction, common::DimsResponsesList >( _action ){};

	virtual void operator()( common::CConnectedNode & _param ) const
	{
		LogPrintf("set response \"connected node\" to action: %p \n", this->m_action );

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

	virtual void operator()( common::CNetworkInfoResult & _param ) const
	{
		LogPrintf("set response \"network info result\" to action: %p \n", this->m_action );
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
		this->m_action->process_event( common::CAckEvent() );
	}

	virtual void operator()( common::CMessageResult & _param ) const
	{
		LogPrintf("set response \"message result\" to action: %p \n", this->m_action );
		this->m_action->process_event( _param );
	}
};

class CSetUpdateDataResult : public CResponseVisitorBase< monitor::CUpdateDataAction, common::DimsResponsesList >
{
public:
	CSetUpdateDataResult( monitor::CUpdateDataAction * const _action ):CResponseVisitorBase< monitor::CUpdateDataAction, common::DimsResponsesList >( _action ){};

	virtual void operator()( common::CMessageResult & _param ) const
	{
		LogPrintf("set response \"message result\" to action: %p \n", this->m_action );
		this->m_action->process_event( _param );
	}

	virtual void operator()( common::CNoMedium & _param ) const
	{
		LogPrintf("set response \"no medium\" to action: %p \n", this->m_action );
		this->m_action->process_event( _param );
	}
};

class CSetRecognizeNetworkResult : public CResponseVisitorBase< monitor::CRecognizeNetworkAction, common::DimsResponsesList >
{
public:
	CSetRecognizeNetworkResult( monitor::CRecognizeNetworkAction * const _action ):CResponseVisitorBase< monitor::CRecognizeNetworkAction, common::DimsResponsesList >( _action ){};

	virtual void operator()( common::ScheduledResult & _param ) const
	{
		boost::apply_visitor( common::CResolveScheduledResult< monitor::CRecognizeNetworkAction >( this->m_action ), _param );
	}
};

class CSetPingResult : public CResponseVisitorBase< monitor::CPingAction, common::DimsResponsesList >
{
public:
	CSetPingResult( monitor::CPingAction * const _action ):CResponseVisitorBase< monitor::CPingAction, common::DimsResponsesList >( _action ){};

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

class CSetAdmitTrackerAction : public CResponseVisitorBase< monitor::CAdmitTrackerAction, common::DimsResponsesList >
{
public:
	CSetAdmitTrackerAction( monitor::CAdmitTrackerAction * const _action ):CResponseVisitorBase< monitor::CAdmitTrackerAction, common::DimsResponsesList >( _action ){};

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
		boost::apply_visitor( common::CResolveScheduledResult< monitor::CAdmitTrackerAction >( this->m_action ), _param );
	}
};

class CSetTrackOriginAddressResult : public CResponseVisitorBase< monitor::CTrackOriginAddressAction, common::DimsResponsesList >
{
public:
	CSetTrackOriginAddressResult( monitor::CTrackOriginAddressAction * const _action ):CResponseVisitorBase< monitor::CTrackOriginAddressAction, common::DimsResponsesList >( _action ){};

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

class CSetProvideInfoActionResult : public CResponseVisitorBase< monitor::CProvideInfoAction, common::DimsResponsesList >
{
public:
	CSetProvideInfoActionResult( monitor::CProvideInfoAction * const _action ):CResponseVisitorBase< monitor::CProvideInfoAction, common::DimsResponsesList >( _action ){};

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
};

class CSetSynchronizationActionResult : public CResponseVisitorBase< monitor::CSynchronizationAction, common::DimsResponsesList >
{
public:
	CSetSynchronizationActionResult( monitor::CSynchronizationAction * const _action ):CResponseVisitorBase< monitor::CSynchronizationAction, common::DimsResponsesList >( _action ){};

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
};

class CSetEnterNetworkActionResult : public CResponseVisitorBase< monitor::CEnterNetworkAction, common::DimsResponsesList >
{
public:
	CSetEnterNetworkActionResult( monitor::CEnterNetworkAction * const _action ):CResponseVisitorBase< monitor::CEnterNetworkAction, common::DimsResponsesList >( _action ){};

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
};

void
CSetResponseVisitor::visit( monitor::CConnectNodeAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< monitor::CConnectNodeAction, common::DimsResponsesList > const &)CSetNodeConnectedResult( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( monitor::CUpdateDataAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< monitor::CUpdateDataAction, common::DimsResponsesList > const &)CSetUpdateDataResult( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( monitor::CAdmitTrackerAction & _action )
{
		boost::apply_visitor( (CResponseVisitorBase< monitor::CAdmitTrackerAction, common::DimsResponsesList > const &)CSetAdmitTrackerAction( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( monitor::CAdmitTransactionBundle & _action )
{
}

void
CSetResponseVisitor::visit( monitor::CPingAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< monitor::CPingAction, common::DimsResponsesList > const &)CSetPingResult( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( monitor::CRecognizeNetworkAction & _action )
{
	boost::apply_visitor( ( CResponseVisitorBase< monitor::CRecognizeNetworkAction, common::DimsResponsesList > const & )CSetRecognizeNetworkResult( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( monitor::CTrackOriginAddressAction & _action )
{
	boost::apply_visitor( ( CResponseVisitorBase< monitor::CTrackOriginAddressAction, common::DimsResponsesList > const & )CSetTrackOriginAddressResult( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( monitor::CProvideInfoAction & _action )
{
	boost::apply_visitor( ( CResponseVisitorBase< monitor::CProvideInfoAction, common::DimsResponsesList > const & )CSetProvideInfoActionResult( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( monitor::CCopyTransactionStorageAction & _action )
{}

void
CSetResponseVisitor::visit( monitor::CSynchronizationAction & _action )
{
	boost::apply_visitor( ( CResponseVisitorBase< monitor::CSynchronizationAction, common::DimsResponsesList > const & )CSetSynchronizationActionResult( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( monitor::CEnterNetworkAction & _action )
{
	boost::apply_visitor( ( CResponseVisitorBase< monitor::CEnterNetworkAction, common::DimsResponsesList > const & )CSetEnterNetworkActionResult( &_action ), m_responses );
}

}

