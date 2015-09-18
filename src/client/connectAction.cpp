// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "connectAction.h"
#include "clientResponses.h"
#include "controlRequests.h"
#include "common/requests.h"

#include "common/setResponseVisitor.h"
#include "common/events.h"

#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/event.hpp>

#include <boost/assign/list_of.hpp>

#include "clientFilters.h"
#include "clientControl.h"
#include "clientEvents.h"

namespace client
{
const unsigned DnsAskLoopTime = 20;//seconds
const unsigned NetworkAskLoopTime = 20;//seconds
const unsigned MonitorAskLoopTime = 20;//seconds

struct CMonitorPresent;
struct CDetermineTrackers;
struct CRecognizeNetwork;

struct CClientUnconnected : boost::statechart::state< CClientUnconnected, CConnectAction >
{
	CClientUnconnected( my_context ctx ) : my_base( ctx )
	{
		CTrackerLocalRanking::getInstance()->resetMonitors();
		CTrackerLocalRanking::getInstance()->resetTrackers();
		context< CConnectAction >().forgetRequests();
		context< CConnectAction >().addRequest( new CDnsInfoRequest() );
	}

	boost::statechart::result react( common::CDnsInfo const & _dnsInfo )
	{
        vector<CAddress> addresses = _dnsInfo.m_addresses;

        if ( addresses.empty() )
		{
            context< CConnectAction >().setExit();
			return discard_event();
		}
		else
		{
            BOOST_FOREACH( CAddress const & address, addresses )
			{
				CTrackerLocalRanking::getInstance()->addUnidentifiedNode( address.ToStringIP(), common::CUnidentifiedNodeInfo( address.ToStringIP(), address.GetPort() ) );
			}
			return transit< CRecognizeNetwork >();
		}
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CDnsInfo >
	> reactions;
};

struct CRecognizeNetwork : boost::statechart::state< CRecognizeNetwork, CConnectAction >
{
	CRecognizeNetwork( my_context ctx ) : my_base( ctx )
	{
		context< CConnectAction >().forgetRequests();

		common::CSendMessageRequest< common::CClientTypes > * request =
				new common::CSendMessageRequest< common::CClientTypes >(
					common::CMainRequestType::NetworkInfoReq
					, new CMediumClassFilter( ClientMediums::Unknown ) );

		context< CConnectAction >().addRequest( request );

		context< CConnectAction >().addRequest( new common::CTimeEventRequest< common::CClientTypes >( NetworkAskLoopTime, new CMediumClassFilter( ClientMediums::Time ) ) );
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		if ( !m_uniqueNodes.size() )
		{
			return transit< CRecognizeNetwork >();
		}

		bool moniorPresent = false;

		analyseData( moniorPresent );

		context< CConnectAction >().forgetRequests();
		return moniorPresent ? transit< CMonitorPresent >() : transit< CDetermineTrackers >();
	}

	boost::statechart::result react( common::CClientNetworkInfoEvent const & _networkInfo )
	{
		m_pending.erase( _networkInfo.m_nodeIndicator );

		common::CNodeInfo nodeStats( _networkInfo.m_selfKey, _networkInfo.m_ip, common::dimsParams().getDefaultClientPort(), common::CRole::Tracker );
		if ( _networkInfo.m_selfRole == common::CRole::Monitor )
		{
			nodeStats.m_role = common::CRole::Monitor;
			CTrackerLocalRanking::getInstance()->addMonitor( common::CMonitorInfo( nodeStats, std::set< CPubKey >()) );// not right
			m_uniqueNodes.insert( nodeStats );
		}
		else if ( _networkInfo.m_selfRole == common::CRole::Tracker )
		{
			CTrackerLocalRanking::getInstance()->addUndeterminedTracker( nodeStats );
			m_uniqueNodes.insert( nodeStats );
		}

		BOOST_FOREACH( common::CValidNodeInfo const & validNode, _networkInfo.m_networkInfo )
		{
			m_uniqueNodes.insert( common::CNodeInfo( validNode.m_key, validNode.m_address.ToStringIP(), common::dimsParams().getDefaultClientPort() ) );
		}

		if ( !m_pending.size() )
		{
			if ( !m_uniqueNodes.size() )
			{
				return transit< CDetermineTrackers >(); // not  ok
			}

			bool moniorPresent = false;

			analyseData( moniorPresent );

			return moniorPresent ? transit< CMonitorPresent >() : transit< CDetermineTrackers >();
		}
		return discard_event();
	}

	boost::statechart::result react( common::CErrorEvent const & _networkInfo )
	{
		context< CConnectAction >().forgetRequests();
		return discard_event();
	}

	void analyseData( bool & _isMonitorPresent )
	{
		_isMonitorPresent = false;

		BOOST_FOREACH( common::CNodeInfo const & validNode, m_uniqueNodes )
		{
			if ( validNode.m_role == common::CRole::Monitor )
			{
				_isMonitorPresent = true;
				CTrackerLocalRanking::getInstance()->addMonitor( common::CMonitorInfo( validNode, std::set< CPubKey >() ) );// this is  not right
			}
			else if ( validNode.m_role == common::CRole::Tracker )
			{
				CTrackerLocalRanking::getInstance()->addUndeterminedTracker( validNode );
			}
		}
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CClientNetworkInfoEvent >,
	boost::statechart::custom_reaction< common::CErrorEvent >
	> reactions;

	// in  future be  careful with  those
	std::set< common::CNodeInfo > m_uniqueNodes;

	std::set< uintptr_t > m_pending;
};


struct CMonitorPresent : boost::statechart::state< CMonitorPresent, CConnectAction >
{
	CMonitorPresent( my_context ctx ) : my_base( ctx )
	{
		context< CConnectAction >().forgetRequests();

		common::CSendMessageRequest< common::CClientTypes > * request =
				new common::CSendMessageRequest< common::CClientTypes >(
					common::CMainRequestType::MonitorInfoReq
					, new CMediumClassFilter( ClientMediums::Monitors ) );

		context< CConnectAction >().addRequest( request );

		context< CConnectAction >().addRequest( new common::CTimeEventRequest< common::CClientTypes >( MonitorAskLoopTime, new CMediumClassFilter( ClientMediums::Time ) ) );
	}
	// try  to  recognize  what  monitors  are  accepted by  which  node
	// determine  network  of  valid monitors
	// next get list  of  recognized  trackers
	// go  to  each  one  an  interrogate  them  next
	//

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		m_checked.insert( m_pending.begin(), m_pending.end() );
		context< CConnectAction >().forgetRequests();

		common::CSendMessageRequest< common::CClientTypes > * request =
				new common::CSendMessageRequest< common::CClientTypes >(
					common::CMainRequestType::MonitorInfoReq
					, new CMediumClassWithExceptionFilter( m_checked, ClientMediums::Monitors ) );

		context< CConnectAction >().addRequest( request );

		return discard_event();
	}

	boost::statechart::result react( common::CNoMedium const & _noMedium )
	{
		analyseAllData();
		std::set< CPubKey > monitors = chooseMonitors();

		CTrackerLocalRanking::getInstance()->resetMonitors();
		CTrackerLocalRanking::getInstance()->resetTrackers();

		BOOST_FOREACH( CPubKey const & key, monitors )
		{
			std::map< CPubKey, std::vector< common::CNodeInfo > >::const_iterator monitorsInfoIterator = m_monitorsInfo.find( key );
			assert( monitorsInfoIterator != m_monitorsInfo.end() );

			BOOST_FOREACH( common::CNodeInfo const & info, monitorsInfoIterator->second )
			{
				std::set< CPubKey > dependentTrackers;
				std::map< CPubKey, std::vector< common::CNodeInfo > >::const_iterator trackers = m_trackersInfo.find( info.m_key );
				if ( trackers != m_trackersInfo.end() )
				{
					BOOST_FOREACH( common::CNodeInfo const & nodeInfo, trackers->second )
					{
						dependentTrackers.insert( nodeInfo.m_key );
					}
				}
				CTrackerLocalRanking::getInstance()->addMonitor( common::CMonitorInfo( info, dependentTrackers ) );
			}

			std::map< CPubKey, std::vector< common::CNodeInfo > >::const_iterator trackersInfoIterator = m_trackersInfo.find( key );

			assert( trackersInfoIterator != m_trackersInfo.end() );

			BOOST_FOREACH( common::CNodeInfo const & info, trackersInfoIterator->second )
			{
				CTrackerLocalRanking::getInstance()->addUndeterminedTracker( info );
			}
		}
		return transit< CDetermineTrackers >();
	}


	struct CSortObject
	{
		CSortObject( set< std::set< CPubKey > >::const_iterator & _iterator, unsigned int _size ):m_iterator( _iterator ),m_size( _size ){}

		bool operator<( CSortObject const & _sortObject ) const
		{
			return m_size < _sortObject.m_size;
		}

		set< std::set< CPubKey > >::const_iterator m_iterator;
		unsigned int m_size;
	};

	boost::statechart::result react( common::CClientMessageResponse const & _message )
	{
		common::CMonitorStatsData monitorStatsData;
		convertClientPayload( _message.m_clientMessage, monitorStatsData );
		m_pending.erase( _message.m_nodePtr );
//load  all  structures
		std::vector< CPubKey > monitorKeys;
		BOOST_FOREACH( common::CNodeInfo const & nodeInfo, monitorStatsData.m_monitorData.m_monitors )
		{
			monitorKeys.push_back( nodeInfo.m_key );

			std::set< CPubKey > dependentTrackers;
			std::map< CPubKey, std::vector< common::CNodeInfo > >::const_iterator trackers = m_trackersInfo.find( nodeInfo.m_key );
			if ( trackers != m_trackersInfo.end() )
			{
				BOOST_FOREACH( common::CNodeInfo const & nodeInfo, trackers->second )
				{
					dependentTrackers.insert( nodeInfo.m_key );
				}
			}
			CTrackerLocalRanking::getInstance()->addMonitor( common::CMonitorInfo( nodeInfo, dependentTrackers ) );
		}
		CPubKey monitorKey;
		CTrackerLocalRanking::getInstance()->getNodeKey( monitorStatsData.m_ip, monitorKey );

	// looks stupid but for sake of algorithm
		monitorKeys.push_back( monitorKey );
		std::vector< common::CNodeInfo > monitorNodeInfo = monitorStatsData.m_monitorData.m_monitors;

		common::CNodeInfo nodeInfo;
		CTrackerLocalRanking::getInstance()->getNodeInfo( monitorKey, nodeInfo );
		monitorNodeInfo.push_back( nodeInfo );

		m_monitorsInfo.insert( std::make_pair( monitorKey, monitorNodeInfo ) );
		m_trackersInfo.insert( std::make_pair( monitorKey, monitorStatsData.m_monitorData.m_trackers ) );

		m_monitorInputData.insert( std::make_pair( monitorKey, monitorKeys ) );

		if ( m_pending.empty() )
		{
			context< CConnectAction >().forgetRequests();

			common::CSendMessageRequest< common::CClientTypes > * request =
					new common::CSendMessageRequest< common::CClientTypes >(
						common::CMainRequestType::MonitorInfoReq
						, new CMediumClassWithExceptionFilter( m_checked, ClientMediums::Monitors ) );

			context< CConnectAction >().addRequest( request );
		}

		// if in  settings  are  some  monitors  addresses they should be used  to get right  network
		return discard_event();
	}

	std::set< CPubKey > chooseMonitors()
	{
		set< std::set< CPubKey > >::const_iterator iterator = m_monitorOutput.begin();
		std::vector< CSortObject > sorted;

		while( iterator != m_monitorOutput.end() )
		{
			sorted.push_back( CSortObject( iterator, iterator->size() ) );
			iterator++;
		}
		std::sort( sorted.begin(), sorted.end() );
		std::reverse( sorted.begin(), sorted.end() );

		vector<CPubKey> const publicKeys = common::dimsParams().getPreferedMonitorsAddresses();

		if ( publicKeys.empty() )
		{
			if ( !sorted.empty() )
			{
				return *sorted.front().m_iterator;
			}
			return std::set< CPubKey >();
		}
		else
		{
			BOOST_FOREACH( CSortObject const & object, sorted )
			{
				BOOST_FOREACH( CPubKey const & pubKey, publicKeys )
				{
					if ( isPresent( *object.m_iterator, pubKey ) )
						return *object.m_iterator;
				}
			}
			return std::set< CPubKey >();
		}

	}

	void analyseAllData()
	{

		BOOST_FOREACH( PAIRTYPE( CPubKey, std::vector< CPubKey > ) const & monitorData, m_monitorInputData )
		{
			CPubKey const & monitor = monitorData.first;
			//m_monitorOutput

			BOOST_FOREACH( PAIRTYPE( CPubKey, std::vector< CPubKey > ) const & monitorReferenceData, m_monitorInputData )
			{
				std::vector< CPubKey > const & validMonitors = monitorReferenceData.second;

				std::vector< CPubKey >::const_iterator isIterator = std::find ( validMonitors.begin(), validMonitors.end(), monitor );

				if ( isPresent( validMonitors, monitor ) && isPresent( monitorData.second, *isIterator ) )
				{
					// admit both
					// this can be done more efficient way but here performence is irrelevant, stick to simplicity
					// don't know  if it is really needed( done this  way at least, but something have to be done it is clear )
					std::vector< std::set< CPubKey > > newOutputGroup;

					if ( m_monitorOutput.empty() )
					{
						std::set< CPubKey > newOutput;

						newOutput.insert( monitor );
						newOutput.insert( *isIterator );
						m_monitorOutput.insert( newOutput );
					}
					else
					{
						// ugly
						std::map< std::set< CPubKey >, std::set< CPubKey > > changeGroup;
						BOOST_FOREACH( std::set< CPubKey > const & output, m_monitorOutput )
						{
							std::set< CPubKey > valid;
							bool firstPresent = false, secondPresent = false;

							BOOST_FOREACH( CPubKey const & outMonitor, output )
							{
								if ( outMonitor == monitor )
								{
									valid.insert( outMonitor );
								}
								else if ( outMonitor == monitorReferenceData.first )
								{
									valid.insert( outMonitor );
								}
								else
								{
									if (
											isPresent( monitorData.second, outMonitor )
											&& isPresent( validMonitors, outMonitor )
											)
									{
										valid.insert( outMonitor );
									}
								}
							}

							if ( valid.size() == output.size() )
							{
								std::set< CPubKey > newOnes;
								if ( !firstPresent )
									newOnes.insert( monitor );
								if ( !secondPresent )
									newOnes.insert( monitorReferenceData.first );

								changeGroup.insert( std::make_pair( output, newOnes ) );
							}
							else
							{
								newOutputGroup.push_back( valid );
							}
						}
					// there are two monitors to be admited, see if there exist suitable list if not create new one
						BOOST_FOREACH( std::set< CPubKey > & newGroup, newOutputGroup )
						{
							m_monitorOutput.insert( newGroup );
						}

						BOOST_FOREACH( PAIRTYPE( std::set< CPubKey >, std::set< CPubKey > ) const & change, changeGroup )
						{
							std::set< CPubKey > changed = change.first;
							m_monitorOutput.erase( change.first );
							changed.insert( change.second.begin(), change.second.end());
							m_monitorOutput.insert( changed );
						}
					}
				}
			}

		}
	}

	template < typename Container, typename Value >
	bool
	isPresent( Container const & _container, Value const & _value )
	{
		return _container.end() != std::find ( _container.begin(), _container.end(), _value );
	}


	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CClientMessageResponse >,
	boost::statechart::custom_reaction< common::CNoMedium >,
	boost::statechart::custom_reaction< common::CTimeEvent >
	> reactions;

	std::map< CPubKey, std::vector< common::CNodeInfo > > m_monitorsInfo;
	std::map< CPubKey, std::vector< common::CNodeInfo > > m_trackersInfo;
	std::set< uintptr_t > m_checked;
	std::set< uintptr_t > m_pending;
	std::map< CPubKey, std::vector< CPubKey > > m_monitorInputData;
	set< std::set< CPubKey > > m_monitorOutput;
};

struct CDetermineTrackers : boost::statechart::state< CDetermineTrackers, CConnectAction >
{
	CDetermineTrackers( my_context ctx ) : my_base( ctx )
	{
		context< CConnectAction >().forgetRequests();

		common::CSendMessageRequest< common::CClientTypes > * request =
				new common::CSendMessageRequest< common::CClientTypes >(
					common::CMainRequestType::TrackerInfoReq
					, new CMediumClassFilter( ClientMediums::UndeterminedTrackers ) );

		context< CConnectAction >().addRequest( request );

		context< CConnectAction >().addRequest( new common::CTimeEventRequest< common::CClientTypes >( NetworkAskLoopTime, new CMediumClassFilter( ClientMediums::Time ) ) );
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		CClientControl::getInstance()->process_event(
					CNetworkDiscoveredEvent(
						  CTrackerLocalRanking::getInstance()->determinedTrackersCount()
						, CTrackerLocalRanking::getInstance()->monitorCount() ) );

		context< CConnectAction >().forgetRequests();
		return discard_event();
	}

	boost::statechart::result react( common::CNoMedium const & _noMedium )
	{
			CClientControl::getInstance()->process_event(
						CNetworkDiscoveredEvent(
							  CTrackerLocalRanking::getInstance()->determinedTrackersCount()
							, CTrackerLocalRanking::getInstance()->monitorCount() ) );

			context< CConnectAction >().m_connected();

			context< CConnectAction >().forgetRequests();
			return discard_event();
	}

	boost::statechart::result react( common::CErrorEvent const & _networkInfo )
	{
		return discard_event();
	}

	boost::statechart::result react( common::CClientMessageResponse const & _message )
	{
		common::CTrackerStatsData trackerStatsData;
		convertClientPayload( _message.m_clientMessage, trackerStatsData );

		common::CNodeInfo undeterminedTracker;
		CTrackerLocalRanking::getInstance()->getUndeterminedTracker( trackerStatsData.m_ip, undeterminedTracker );

		common::CTrackerStats trackerStats(
			  undeterminedTracker.m_key
			, 0
			, trackerStatsData.m_price
			, undeterminedTracker.m_ip
			, undeterminedTracker.m_port
			);

		CTrackerLocalRanking::getInstance()->addTracker( trackerStats );

		CTrackerLocalRanking::getInstance()->removeUndeterminedTracker( trackerStatsData.m_ip );

		m_pending.erase( trackerStatsData.m_nodeIndicator );

		if ( !m_pending.size() )
		{
			CClientControl::getInstance()->process_event(
						CNetworkDiscoveredEvent(
							  CTrackerLocalRanking::getInstance()->determinedTrackersCount()
							, CTrackerLocalRanking::getInstance()->monitorCount() ) );

			context< CConnectAction >().m_connected();

			context< CConnectAction >().forgetRequests();
		}

		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CNoMedium >,
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CClientMessageResponse >
	> reactions;

	std::set< uintptr_t > m_pending;

	int64_t m_lastAskTime;

	std::map< uintptr_t, uint256 > m_nodeToToken;
};

CConnectAction::CConnectAction( bool _autoDelete )
	: common::CAction< common::CClientTypes >( _autoDelete )
{
	initiate();
}

void
CConnectAction::accept( common::CSetResponseVisitor< common::CClientTypes > & _visitor )
{
	_visitor.visit( *this );
}

bool
CConnectAction::isRequestReady() const
{
	return true;
}

void
CConnectAction::reset()
{
	common::CAction< common::CClientTypes >::reset();
	initiate();
}
}
