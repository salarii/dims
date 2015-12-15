// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "informationProvider.h"
#include <boost/assign.hpp>
#include <boost/foreach.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/thread.hpp>

#include <sstream>

#include "client/trackerLocalRanking.h"
#include "qt/dimsUnits.h"

#include "base58.h"

using boost::assign::map_list_of;
using namespace boost::assign;
using namespace boost;
using namespace monitorsScaner;

template < typename ConvertedType >
std::string
convert( ConvertedType const & _converted )
{
	std::ostringstream stream;
	stream << _converted;
	return stream.str();
}

CInforamtionProvider * CInforamtionProvider::ms_instance = NULL;

CInforamtionProvider*
CInforamtionProvider::getInstance( )
{
	if ( !ms_instance )
	{
		ms_instance = new CInforamtionProvider();
	};
	return ms_instance;
}


CInforamtionProvider::CInforamtionProvider()
{
}

void
CInforamtionProvider::reloadData()
{
	m_trackers.clear();
	m_usedMonitorsTest.clear();
	std::vector< common::CTrackerStats > trackers = client::CTrackerLocalRanking::getInstance()->getTrackers();
	BOOST_FOREACH( common::CTrackerStats const & tracker, trackers )
	{

		CNodeAddress node;

		std::string monitor;
		CPubKey monitorKey;
		if ( client::CTrackerLocalRanking::getInstance()->getMonitorKeyForTracker( tracker.m_key, monitorKey ) )
		{
			node.Set( monitorKey.GetID(), common::NodePrefix::Monitor );
			monitor = node.ToString();
		}

		node.Set( tracker.m_key.GetID(), common::NodePrefix::Tracker );

		m_trackers.insert(
					std::make_pair(
						monitor
						, ( std::list<std::string> const & )boost::assign::list_of( tracker.m_ip )( node.ToString() )( "" )( convert( tracker.m_reputation ) )( CDimsUnits::format(0, tracker.m_price).toStdString() )
						)
				);
	}

	std::vector< common::CMonitorInfo > monitors = client::CTrackerLocalRanking::getInstance()->getMonitors();
	BOOST_FOREACH( common::CNodeInfo const & monitor, monitors )
	{
		CNodeAddress monitorAddress;

		monitorAddress.Set( monitor.m_key.GetID(), common::NodePrefix::Monitor );
		m_usedMonitorsTest.push_back( boost::assign::list_of( monitor.m_ip )( monitorAddress.ToString() )("")("")("") );
	}
}



void
CInforamtionProvider::getTrackers(monitorsScaner::Data& _trackers, std::string _publicKey ) const
{
	typedef std::multimap< std::string ,std::list<std::string> >::const_iterator TrackerInfoIterator;

	unsigned int rows = 0, columns = 0;
	 std::pair<  TrackerInfoIterator, TrackerInfoIterator> keyRange= m_trackers.equal_range(_publicKey);
	// Iterate over all map elements with key == theKey


	for (TrackerInfoIterator it = keyRange.first;  it != keyRange.second;  ++it)
	{
		rows++;
		columns = it->second.size();

		BOOST_FOREACH( std::string const & info, it->second )
		{
			_trackers.data.push_back( info );
		};
	}

	_trackers.cols = columns;
	_trackers.rows = rows;
}

typedef std::list<std::string> StringStorage;

void
CInforamtionProvider::getMonitorsInfo(monitorsScaner::Data& _monitors) const
{

	unsigned int rows = 0, columns = 0;
	BOOST_FOREACH( StringStorage const & storage, m_usedMonitors )
	{
		rows++;
		columns = storage.size();
		BOOST_FOREACH( std::string const &data, storage )
		{
			_monitors.data.push_back( data );
		}
	}

	_monitors.cols = columns;
	_monitors.rows = rows;
}

void 
CInforamtionProvider::getInfo(monitorsScaner::Data& _return, const InfoRequest& infoRequest)
{
	if ( Info::TRACKERS_INFO == infoRequest.info )
	{	
		getTrackers(_return, infoRequest.key );
	}
	else if( Info::MONITORS_INFO == infoRequest.info  )
	{
		if ( infoRequest.networkType == NetworkType::TESTNET )
		{
			m_usedMonitors = m_usedMonitorsTest;
		}
		else if ( infoRequest.networkType == NetworkType::MAIN )
		{
			m_usedMonitors = m_usedMonitorsMain;
		}
		else
		{
			Exception e;
			e.msg = "specify network type";
			throw e;
		}
		getMonitorsInfo(_return);
	}
	else 
	{
		Exception e;

		e.msg = "specify tracker or monitor info";
		throw e;
	}
}
