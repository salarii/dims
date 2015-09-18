// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/setResponseVisitor.h"
#include "common/responseVisitorInternal.h"
#include "common/events.h"

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

class GetBalance : public CResponseVisitorBase< tracker::CGetBalanceAction, common::DimsResponsesList >
{
public:
	GetBalance( tracker::CGetBalanceAction * const _action ):CResponseVisitorBase< tracker::CGetBalanceAction, common::DimsResponsesList >( _action ){};

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

class CSetValidationResult : public CResponseVisitorBase< tracker::CValidateTransactionsAction, common::DimsResponsesList >
{
public:
	CSetValidationResult( tracker::CValidateTransactionsAction * const _action ):CResponseVisitorBase< tracker::CValidateTransactionsAction, common::DimsResponsesList >( _action ){};

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

class CSetNodeConnectedResult : public CResponseVisitorBase< tracker::CConnectNodeAction, common::DimsResponsesList >
{
public:
	CSetNodeConnectedResult( tracker::CConnectNodeAction * const _action ):CResponseVisitorBase< tracker::CConnectNodeAction, common::DimsResponsesList >( _action ){};

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

class CSetTrackOriginAddressResult : public CResponseVisitorBase< tracker::CTrackOriginAddressAction, common::DimsResponsesList >
{
public:
	CSetTrackOriginAddressResult( tracker::CTrackOriginAddressAction * const _action ):CResponseVisitorBase< tracker::CTrackOriginAddressAction, common::DimsResponsesList >( _action ){};

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

class CSetSynchronizationResult : public CResponseVisitorBase< tracker::CSynchronizationAction, common::DimsResponsesList >
{
public:
	CSetSynchronizationResult( tracker::CSynchronizationAction * const _action ):CResponseVisitorBase< tracker::CSynchronizationAction, common::DimsResponsesList >( _action ){};

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

class CSetRecognizeNetworkResult : public CResponseVisitorBase< tracker::CRecognizeNetworkAction, common::DimsResponsesList >
{
public:
	CSetRecognizeNetworkResult( tracker::CRecognizeNetworkAction * const _action ):CResponseVisitorBase< tracker::CRecognizeNetworkAction, common::DimsResponsesList >( _action ){};

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

class CSetConnectNetworkResult : public CResponseVisitorBase< tracker::CConnectNetworkAction, common::DimsResponsesList >
{
public:
	CSetConnectNetworkResult( tracker::CConnectNetworkAction * const _action ):CResponseVisitorBase< tracker::CConnectNetworkAction, common::DimsResponsesList >( _action ){};

	virtual void operator()( common::ScheduledResult & _param ) const
	{
		LogPrintf("set response \"schedule result\" to action: %p \n", this->m_action );
		boost::apply_visitor( common::CResolveScheduledResult< tracker::CConnectNetworkAction >( this->m_action ), _param );
	}
};

class CSetProvideInfoResult : public CResponseVisitorBase< tracker::CProvideInfoAction, common::DimsResponsesList >
{
public:
	CSetProvideInfoResult( tracker::CProvideInfoAction * const _action ):CResponseVisitorBase< tracker::CProvideInfoAction, common::DimsResponsesList >( _action ){};

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

class CSetPingResult : public CResponseVisitorBase< tracker::CPingAction, common::DimsResponsesList >
{
public:
	CSetPingResult( tracker::CPingAction * const _action ):CResponseVisitorBase< tracker::CPingAction, common::DimsResponsesList >( _action ){};

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

class CSetPassTransactionResult : public CResponseVisitorBase< tracker::CPassTransactionAction, common::DimsResponsesList >
{
public:
	CSetPassTransactionResult( tracker::CPassTransactionAction * const _action ):CResponseVisitorBase< tracker::CPassTransactionAction, common::DimsResponsesList >( _action ){};

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

class CSetRegisterAction : public CResponseVisitorBase< tracker::CRegisterAction, common::DimsResponsesList >
{
public:
	CSetRegisterAction( tracker::CRegisterAction * const _action ):CResponseVisitorBase< tracker::CRegisterAction, common::DimsResponsesList >( _action ){};

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

void
CSetResponseVisitor::visit( tracker::CGetBalanceAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< tracker::CGetBalanceAction, common::DimsResponsesList > const &)GetBalance( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( tracker::CValidateTransactionsAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< tracker::CValidateTransactionsAction, common::DimsResponsesList > const &)CSetValidationResult( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( tracker::CConnectNodeAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< tracker::CConnectNodeAction, common::DimsResponsesList > const &)CSetNodeConnectedResult( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( tracker::CTrackOriginAddressAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< tracker::CTrackOriginAddressAction, common::DimsResponsesList > const &)CSetTrackOriginAddressResult( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( tracker::CSynchronizationAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< tracker::CSynchronizationAction, common::DimsResponsesList > const &)CSetSynchronizationResult( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( tracker::CProvideInfoAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< tracker::CProvideInfoAction, common::DimsResponsesList > const &)CSetProvideInfoResult( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( tracker::CPingAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< tracker::CPingAction, common::DimsResponsesList > const &)CSetPingResult( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( tracker::CRegisterAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< tracker::CRegisterAction, common::DimsResponsesList > const &)CSetRegisterAction( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( tracker::CRecognizeNetworkAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< tracker::CRecognizeNetworkAction, common::DimsResponsesList > const &)CSetRecognizeNetworkResult( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( tracker::CConnectNetworkAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< tracker::CConnectNetworkAction, common::DimsResponsesList > const &)CSetConnectNetworkResult( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( tracker::CPassTransactionAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< tracker::CPassTransactionAction, common::DimsResponsesList > const &)CSetPassTransactionResult( &_action ), m_responses );
}

}
