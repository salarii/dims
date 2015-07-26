// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/setResponseVisitor.h"
#include "common/responseVisitorInternal.h"
#include "common/commonEvents.h"

#include "monitor/connectNodeAction.h"
#include "monitor/updateDataAction.h"
#include "monitor/admitTrackerAction.h"
#include "monitor/admitTransactionsBundle.h"
#include "monitor/pingAction.h"
#include "monitor/recognizeNetworkAction.h"
#include "monitor/trackOriginAddressAction.h"
#include "monitor/provideInfoAction.h"

namespace common
{

class CSetNodeConnectedResult : public CResponseVisitorBase< monitor::CConnectNodeAction, monitor::MonitorResponseList >
{
public:
	CSetNodeConnectedResult( monitor::CConnectNodeAction * const _action ):CResponseVisitorBase< monitor::CConnectNodeAction, monitor::MonitorResponseList >( _action ){};

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
		this->m_action->process_event( common::CNetworkInfoEvent( _param.m_nodeSelfInfo, _param.m_role, _param.m_trackersInfo, _param.m_monitorsInfo ) );
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

	virtual void operator()( common::CGetPrompt & _param ) const
	{
		LogPrintf("set response \"get prompt\" to action: %p \n", this->m_action );
		this->m_action->process_event( common::CGetEvent(_param.m_type) );
	}
};

class CSetUpdateDataResult : public CResponseVisitorBase< monitor::CUpdateDataAction, monitor::MonitorResponseList >
{
public:
	CSetUpdateDataResult( monitor::CUpdateDataAction * const _action ):CResponseVisitorBase< monitor::CUpdateDataAction, monitor::MonitorResponseList >( _action ){};

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

class CSetRecognizeNetworkResult : public CResponseVisitorBase< monitor::CRecognizeNetworkAction, monitor::MonitorResponseList >
{
public:
	class CResolveNetworkResult : public boost::static_visitor< void >
	{
	public:
		CResolveNetworkResult( monitor::CRecognizeNetworkAction * const _action)
			: m_action( _action )
		{}

		void operator()( CNetworkInfoResult const & _networkInfoResult ) const
		{
			m_action->process_event( common::CNetworkInfoEvent( _networkInfoResult.m_nodeSelfInfo, _networkInfoResult.m_role, _networkInfoResult.m_trackersInfo, _networkInfoResult.m_monitorsInfo ) );
		}

		void operator()( CTransaction const & ) const
		{
		}

		void operator()( CValidRegistration const & ) const
		{
		}
	private:
		monitor::CRecognizeNetworkAction * m_action;
	};

public:
	CSetRecognizeNetworkResult( monitor::CRecognizeNetworkAction * const _action ):CResponseVisitorBase< monitor::CRecognizeNetworkAction, monitor::MonitorResponseList >( _action ){};

	virtual void operator()( common::ScheduledResult & _param ) const
	{
		boost::apply_visitor( CResolveNetworkResult( this->m_action ), _param );
	}
};

class CSetPingResult : public CResponseVisitorBase< monitor::CPingAction, monitor::MonitorResponseList >
{
public:
	CSetPingResult( monitor::CPingAction * const _action ):CResponseVisitorBase< monitor::CPingAction, monitor::MonitorResponseList >( _action ){};

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

class CSetAdmitTrackerAction : public CResponseVisitorBase< monitor::CAdmitTrackerAction, monitor::MonitorResponseList >
{
public:
	CSetAdmitTrackerAction( monitor::CAdmitTrackerAction * const _action ):CResponseVisitorBase< monitor::CAdmitTrackerAction, monitor::MonitorResponseList >( _action ){};

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

class CSetTrackOriginAddressResult : public CResponseVisitorBase< monitor::CTrackOriginAddressAction, monitor::MonitorResponseList >
{
public:
	CSetTrackOriginAddressResult( monitor::CTrackOriginAddressAction * const _action ):CResponseVisitorBase< monitor::CTrackOriginAddressAction, monitor::MonitorResponseList >( _action ){};

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

class CSetProvideInfoActionResult : public CResponseVisitorBase< monitor::CProvideInfoAction, monitor::MonitorResponseList >
{
public:
	CSetProvideInfoActionResult( monitor::CProvideInfoAction * const _action ):CResponseVisitorBase< monitor::CProvideInfoAction, monitor::MonitorResponseList >( _action ){};

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

CSetResponseVisitor< common::CMonitorTypes >::CSetResponseVisitor( monitor::MonitorResponses const & _requestResponse )
	: m_requestResponse( _requestResponse )
{
}

void
CSetResponseVisitor< common::CMonitorTypes >::visit( monitor::CConnectNodeAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< monitor::CConnectNodeAction, monitor::MonitorResponseList > const &)CSetNodeConnectedResult( &_action ), m_requestResponse );
}

void
CSetResponseVisitor< common::CMonitorTypes >::visit( monitor::CUpdateDataAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< monitor::CUpdateDataAction, monitor::MonitorResponseList > const &)CSetUpdateDataResult( &_action ), m_requestResponse );
}

void
CSetResponseVisitor< common::CMonitorTypes >::visit( monitor::CAdmitTrackerAction & _action )
{
		boost::apply_visitor( (CResponseVisitorBase< monitor::CAdmitTrackerAction, monitor::MonitorResponseList > const &)CSetAdmitTrackerAction( &_action ), m_requestResponse );
}

void
CSetResponseVisitor< common::CMonitorTypes >::visit( monitor::CAdmitProofTransactionBundle & _action )
{
}

void
CSetResponseVisitor< common::CMonitorTypes >::visit( monitor::CPingAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< monitor::CPingAction, monitor::MonitorResponseList > const &)CSetPingResult( &_action ), m_requestResponse );
}

void
CSetResponseVisitor< common::CMonitorTypes >::visit( monitor::CRecognizeNetworkAction & _action )
{
	boost::apply_visitor( ( CResponseVisitorBase< monitor::CRecognizeNetworkAction, monitor::MonitorResponseList > const & )CSetRecognizeNetworkResult( &_action ), m_requestResponse );
}

void
CSetResponseVisitor< common::CMonitorTypes >::visit( monitor::CTrackOriginAddressAction & _action )
{
	boost::apply_visitor( ( CResponseVisitorBase< monitor::CTrackOriginAddressAction, monitor::MonitorResponseList > const & )CSetTrackOriginAddressResult( &_action ), m_requestResponse );
}

void
CSetResponseVisitor< common::CMonitorTypes >::visit( monitor::CProvideInfoAction & _action )
{
	boost::apply_visitor( ( CResponseVisitorBase< monitor::CProvideInfoAction, monitor::MonitorResponseList > const & )( &_action ), m_requestResponse );
}

void
CSetResponseVisitor< common::CMonitorTypes >::visit( monitor::CCopyTransactionStorageAction & _action )
{}

}

