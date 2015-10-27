// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef COMMON_EVENTS_H
#define COMMON_EVENTS_H

#include <boost/statechart/event.hpp>

#include "net.h"
#include "main.h"

#include "common/responses.h"

namespace common
{

class CSelfNode;

struct CNodeConnectedEvent : boost::statechart::event< CNodeConnectedEvent >
{
	CNodeConnectedEvent( common::CSelfNode * _node ):m_node( _node ){};
	common::CSelfNode * m_node;
};

struct CCantReachNode : boost::statechart::event< CCantReachNode >
{
};

struct CRoleEvent : boost::statechart::event< CRoleEvent >
{
	CRoleEvent( int _role ):m_role( _role ){};
	int m_role;
};

struct CNetworkRecognizedData : boost::statechart::event< CNetworkRecognizedData >
{
	CNetworkRecognizedData(){};
	CNetworkRecognizedData( std::set< CValidNodeInfo > const & _trackersInfo, std::set< CValidNodeInfo > const & _monitorsInfo ):m_trackersInfo( _trackersInfo ),m_monitorsInfo( _monitorsInfo ){};
	std::set< CValidNodeInfo > m_trackersInfo;
	std::set< CValidNodeInfo > m_monitorsInfo;
};

struct CClientNetworkInfoEvent : boost::statechart::event< CClientNetworkInfoEvent >
{
	CClientNetworkInfoEvent( std::vector< CValidNodeInfo > const & _networkInfo, CPubKey const & _selfKey, int _selfRole ):m_networkInfo( _networkInfo ),m_selfKey( _selfKey ), m_selfRole( _selfRole ), m_ip(), m_nodeIndicator( 0 ){};
	CClientNetworkInfoEvent( std::vector< CValidNodeInfo > const & _networkInfo, CPubKey const & _selfKey, int _selfRole , std::string _ip, uintptr_t _nodeIndicator ):m_networkInfo( _networkInfo ),m_selfKey( _selfKey ), m_selfRole( _selfRole ), m_ip( _ip ), m_nodeIndicator( _nodeIndicator ){};
	std::vector< CValidNodeInfo > m_networkInfo;
	CPubKey m_selfKey;
	int m_selfRole;
	std::string m_ip;
	uintptr_t m_nodeIndicator;
};

struct CRegistrationData : boost::statechart::event< CRegistrationData >
{
	CRegistrationData(){}

	CRegistrationData( CPubKey const & _key, uint64_t _registrationTime, uint64_t const & _period )
		: m_key( _key )
		, m_registrationTime( _registrationTime )
		, m_period( _period )
	{}

	CPubKey m_key;
	uint64_t m_registrationTime;
	uint64_t m_period;
};

struct CResultEvent : boost::statechart::event< CResultEvent >
{
	CResultEvent( bool _result ):m_result( _result ){}

	unsigned int m_result;
};

struct CErrorEvent : boost::statechart::event< CErrorEvent >
{
};

struct CSwitchToConnectedEvent : boost::statechart::event< CSwitchToConnectedEvent >
{
};

struct CSwitchToConnectingEvent : boost::statechart::event< CSwitchToConnectingEvent >
{
};


// there  will be  negative  and  positive  ack,  because  ack  acts  as  approval  sometimes
struct CAckEvent : boost::statechart::event< CAckEvent >
{
	CAckEvent(){};
	CAckEvent( uintptr_t _nodePtr ):m_nodePtr( _nodePtr )
	{}

	uintptr_t m_nodePtr;
};

struct CStartPingEvent : boost::statechart::event< CStartPingEvent >
{
};

struct CStartPongEvent : boost::statechart::event< CStartPongEvent >
{
};

struct CInitialSynchronizationDoneEvent : boost::statechart::event< CInitialSynchronizationDoneEvent >
{
};

struct CMerkleBlocksEvent : boost::statechart::event< CMerkleBlocksEvent >
{
	CMerkleBlocksEvent( std::vector< CMerkleBlock > const & _merkles, std::map< uint256 ,std::vector< CTransaction > > const & _transactions, uintptr_t _nodePtr )
		: m_merkles( _merkles ), m_transactions( _transactions ), m_nodePtr( _nodePtr ){};
	std::vector< CMerkleBlock > m_merkles;
	std::map< uint256 ,std::vector< CTransaction > > m_transactions;
	uintptr_t const m_nodePtr;
};

struct CUpdateStatus : boost::statechart::event< CUpdateStatus >
{
};

struct CSynchronizeBitcoinAsk : boost::statechart::event< CSynchronizeBitcoinAsk >
{
};

struct CSetScanBitcoinChainProgress : boost::statechart::event< CSetScanBitcoinChainProgress >
{
	CSetScanBitcoinChainProgress( int _blockLeft ):m_blockLeft( _blockLeft ){}

	int m_blockLeft;
};

struct CBitcoinNetworkConnection : boost::statechart::event< CBitcoinNetworkConnection >
{
	CBitcoinNetworkConnection( int _nodesNumber ):m_nodesNumber( _nodesNumber ){}

	int m_nodesNumber;
};

struct CTransactionAckEvent : boost::statechart::event< CTransactionAckEvent >
{
	CTransactionAckEvent( common::TransactionsStatus::Enum _status, CTransaction _transactionSend ): m_status( _status ), m_transactionSend( _transactionSend ){}
	common::TransactionsStatus::Enum m_status;
	CTransaction m_transactionSend;
};

struct CRankingEvent : boost::statechart::event< CRankingEvent >
{
	CRankingEvent( CRankingFullInfo const & _rankingInfo )
		: m_rankingInfo( _rankingInfo )
	{}

	CRankingFullInfo m_rankingInfo;
};

struct CRegistrationTermsEvent : boost::statechart::event< CRegistrationTermsEvent >
{
	CRegistrationTermsEvent( CRegistrationTerms const & _registrationTerms )
		: m_registrationTerms( _registrationTerms )
	{}

	CRegistrationTerms m_registrationTerms;
};

struct CTrackerInfoEvent : boost::statechart::event< CTrackerInfoEvent >
{
	CTrackerInfoEvent( CTrackerInfo const & _trackerInfo )
		: m_trackerInfo( _trackerInfo )
	{}

	CTrackerInfo m_trackerInfo;
};

template < class Action >
class CResolveScheduledResult : public boost::static_visitor< void >
{
public:
	CResolveScheduledResult( Action * const _action)
		: m_action( _action )
	{}

	void operator()( CNetworkInfoResult const & _networkInfoResult ) const
	{
		m_action->process_event( _networkInfoResult );
	}

	void operator()( CTransactionAck const & _transactionAck ) const
	{
		m_action->process_event( common::CTransactionAckEvent( ( common::TransactionsStatus::Enum )_transactionAck.m_status, _transactionAck.m_transaction ) );
	}

	void operator()( CSynchronizationResult const & _synchronizationResult ) const
	{
		this->m_action->process_event( _synchronizationResult );
	}

	void operator()( CValidRegistration const & _validRegistration ) const
	{
		this->m_action->process_event( common::CRegistrationData( _validRegistration.m_key, _validRegistration.m_registrationTime, _validRegistration.m_period ) );
	}

	void operator()( CResult const & _result ) const
	{
		this->m_action->process_event( CResultEvent( _result.m_result ) );
	}

	void operator()( CTrackerInfo const & _trackerInfo ) const
	{
		this->m_action->process_event( CTrackerInfoEvent( _trackerInfo ) );
	}

	void operator()( CExecutedIndicator const & _executedIndicator ) const
	{
		this->m_action->process_event( _executedIndicator );
	}

	void operator()( CAvailableCoinsData const & _availableCoins ) const
	{
		this->m_action->process_event( _availableCoins );
	}

	void operator()( CFailureEvent const & _failureEvent ) const
	{
		this->m_action->process_event( _failureEvent );
	}

	void operator()( CRankingFullInfo const & _rankingInfo ) const
	{
		this->m_action->process_event( CRankingEvent( _rankingInfo ) );
	}

	void operator()( CRegistrationTerms const & _registrationTerms ) const
	{
		this->m_action->process_event( CRegistrationTermsEvent( _registrationTerms ) );
	}
private:
	Action * m_action;
};

struct CValidationEvent : boost::statechart::event< CValidationEvent >
{
	CValidationEvent( std::vector< unsigned int > const & _invalidTransactionIndexes ):m_invalidTransactionIndexes( _invalidTransactionIndexes ){};
	std::vector< unsigned int > m_invalidTransactionIndexes;
};

}

#endif // COMMON_EVENTS_H
