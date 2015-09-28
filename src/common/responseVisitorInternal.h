#ifndef RESPONSE_VISITOR_INTERNAL_H
#define RESPONSE_VISITOR_INTERNAL_H

#include <boost/variant.hpp>

#include "common/events.h"
#include "common/visitorConfigurationUtilities.h"

namespace common
{

template < class _Action, typename _ParamList >
class CResponseVisitorBase : public boost::static_visitor< void >
{
public:
	CResponseVisitorBase( _Action * const _action ):m_action( _action ){};

	virtual void operator()(typename VisitorParam(  _ParamList, 0 ) & _param ) const
	{
	}

	virtual void operator()(typename VisitorParam( _ParamList, 1 ) & _param ) const
	{
	}

	virtual void operator()(typename VisitorParam( _ParamList, 2 ) & _param ) const
	{
	}

	virtual void operator()(typename VisitorParam( _ParamList, 3 ) & _param ) const
	{
	}

	virtual void operator()(typename VisitorParam( _ParamList, 4 ) & _param ) const
	{
	}

	virtual void operator()(typename VisitorParam( _ParamList, 5 ) & _param ) const
	{
	}

	virtual void operator()(typename VisitorParam( _ParamList, 6 ) & _param ) const
	{
	}

	virtual void operator()(typename VisitorParam( _ParamList, 7 ) & _param ) const
	{
	}

	virtual void operator()(typename VisitorParam( _ParamList, 8 ) & _param ) const
	{
	}

	virtual void operator()(typename VisitorParam( _ParamList, 9 ) & _param ) const
	{
	}

	virtual void operator()(typename VisitorParam( _ParamList, 10 ) & _param ) const
	{
	}

	virtual void operator()(typename VisitorParam( _ParamList, 11 ) & _param ) const
	{
	}

	virtual void operator()(typename VisitorParam( _ParamList, 12 ) & _param ) const
	{
	}

	virtual void operator()(typename VisitorParam( _ParamList, 13 ) & _param ) const
	{
	}

	virtual void operator()(typename VisitorParam( _ParamList, 14 ) & _param ) const
	{
	}

	virtual void operator()(typename VisitorParam( _ParamList, 15 ) & _param ) const
	{
	}

	virtual void operator()(typename VisitorParam( _ParamList, 16 ) & _param ) const
	{
	}

	virtual void operator()(typename VisitorParam( _ParamList, 17 ) & _param ) const
	{
	}

	virtual void operator()(typename VisitorParam( _ParamList, 18 ) & _param ) const
	{
	}

	virtual void operator()(typename VisitorParam( _ParamList, 19 ) & _param ) const
	{
	}

protected:
	_Action * const m_action;
};

template < class _Action >
class CSetResult : public CResponseVisitorBase< _Action, common::DimsResponsesList >
{
public:
	CSetResult( _Action * const _action ):CResponseVisitorBase< _Action, common::DimsResponsesList >( _action ){};

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

	virtual void operator()( common::CRequestedMerkles & _param ) const
	{
		LogPrintf("set response \"requested merkles\" to action: %p \n", this->m_action );
		this->m_action->process_event( common::CMerkleBlocksEvent( _param.m_merkles, _param.m_transactions, _param.m_nodePtr ) );
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

	virtual void operator()( common::CNoMedium & _param ) const
	{
		LogPrintf("set response \"no medium\" to action: %p \n", this->m_action );
		this->m_action->process_event( _param );
	}

	virtual void operator()( common::ScheduledResult & _param ) const
	{
		LogPrintf("set response \"schedule result\" to action: %p \n", this->m_action );
		boost::apply_visitor( common::CResolveScheduledResult< _Action >( this->m_action ), _param );
	}
};

}

#endif // RESPONSE_VISITOR_INTERNAL_H
