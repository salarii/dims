// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef RESPONSE_VISITOR_INTERNAL_H
#define RESPONSE_VISITOR_INTERNAL_H

#include <boost/variant.hpp>

#include "common/events.h"
#include "common/visitorConfigurationUtilities.h"

namespace common
{

template < class _Action >
class CSetResult : public boost::static_visitor< void >
{
public:
	CSetResult( _Action * const _action ):m_action( _action ){};

	virtual void operator()( common::CConnectedNode & _param ) const
	{
		LogPrintf("set response \"connected node\" to action: %p \n", m_action );

		if ( _param.m_node )
			m_action->process_event( common::CNodeConnectedEvent( _param.m_node ) );
		else
			m_action->process_event( common::CCantReachNode() );
	}

	virtual void operator()( common::CIdentificationResult & _param ) const
	{
		LogPrintf("set response \"identification result\" to action: %p \n", m_action );
		m_action->process_event( _param );
	}

	virtual void operator()( common::CRequestedMerkles & _param ) const
	{
		LogPrintf("set response \"requested merkles\" to action: %p \n", m_action );
		m_action->process_event( common::CMerkleBlocksEvent( _param.m_merkles, _param.m_transactions, _param.m_nodePtr ) );
	}

	virtual void operator()( common::CNetworkInfoResult & _param ) const
	{
		LogPrintf("set response \"network info result\" to action: %p \n", m_action );
		m_action->process_event( _param );
	}

	virtual void operator()( common::CTimeEvent & _param ) const
	{
		LogPrintf("set response \"time event\" to action: %p \n", m_action );
		m_action->process_event( _param );
	}

	virtual void operator()( common::CAvailableCoinsData & _param ) const
	{
		LogPrintf("set response \"available coins\" to action: %p \n", m_action );
		m_action->process_event( _param );
	}

	void operator()(common::CMediumException & _param ) const
	{
		LogPrintf("set response \"system error\" to action: %p \n", m_action );
		m_action->process_event( common::CErrorEvent() );
	}

	virtual void operator()( common::CAckResult & _param ) const
	{
		LogPrintf("set response \"ack\" to action: %p \n", m_action );
		m_action->process_event( common::CAckEvent() );
	}

	virtual void operator()( common::CMessageResult & _param ) const
	{
		LogPrintf("set response \"message result\" to action: %p \n", m_action );
		m_action->process_event( _param );
	}

	virtual void operator()( common::CNoMedium & _param ) const
	{
		LogPrintf("set response \"no medium\" to action: %p \n", m_action );
		m_action->process_event( _param );
	}

	virtual void operator()( common::ScheduledResult & _param ) const
	{
		LogPrintf("set response \"schedule result\" to action: %p \n", m_action );
		boost::apply_visitor( common::CResolveScheduledResult< _Action >( m_action ), _param );
	}

	virtual void operator()( common::CDnsInfo & _param ) const
	{
		LogPrintf("set response \"dns info\" to action: %p \n", m_action );
		m_action->process_event( _param );
	}

	virtual void operator()( common::CClientMessageResponse & _param ) const
	{
		LogPrintf("set response \"client message response\" to action: %p \n", m_action );
		m_action->process_event( _param );
	}

	virtual void operator()( common::CValidationResult & _param ) const
	{
		LogPrintf("set response \"validation result\" to action: %p \n", m_action );
		m_action->process_event( common::CValidationEvent( _param.m_invalidTransactionIndexes ) );
	}

	virtual void operator()( common::CFailureEvent & _param ) const
	{
		LogPrintf("set response \"failure event\" to action: %p \n", m_action );
		m_action->process_event( _param );
	}
protected:
	_Action * const m_action;

};

}

#endif // RESPONSE_VISITOR_INTERNAL_H
