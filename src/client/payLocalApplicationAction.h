// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PAY_LOCAL_APPLICATION_ACTION_H
#define PAY_LOCAL_APPLICATION_ACTION_H

#include <boost/optional.hpp>

#include <boost/statechart/state_machine.hpp>

#include "core.h"

#include "common/action.h"
#include "common/request.h"
#include "common/transactionStatus.h"
#include "common/responses.h"

namespace common
{

class CSetResponseVisitor;

}

namespace client
{

struct CCheckAppData;

class CPayLocalApplicationAction : public common::CAction, public  boost::statechart::state_machine< CPayLocalApplicationAction, CCheckAppData >
{
public:
	CPayLocalApplicationAction( uintptr_t _socket, CPrivKey const & _privateKey, CKeyID const & _targetKey, int64_t _value,std::vector<CKeyID> const & _trackers, std::vector<CKeyID> const & _monitors );

	void accept( common::CSetResponseVisitor & _visitor );

	CPrivKey getPrivAppKey() const;

	std::vector<CKeyID> const & getTrackers() const;

	std::vector<CKeyID> const & getMonitors() const;

	CKey getPrivKey() const;

	CKeyID getTargetKey() const;

	int64_t getValue() const;

	uintptr_t getSocket() const;

	void setFirstTransaction( CTransaction const & _firstTransaction );

	CTransaction const & getFirstTransaction() const;

	void setSecondTransaction( CTransaction const & _secondTransaction );

	CTransaction const & getSecondTransaction() const;

	void setTrackerStats( common::CTrackerStats const & _trackerStats );

	common::CTrackerStats getTrackerStats() const;

	void setServicingTracker( CPubKey const & _pubKey );

	CPubKey const & getServicingTracker() const;

	void setFirstInitialHash( uint256 const & _firstInitialHash )
	{
		m_firstInitialHash = _firstInitialHash;
	}
	uint256 getFirstInitailHash() const
	{
		return m_firstInitialHash;
	}

	void setMonitorData( common::CMonitorData const & _monitorData )
	{
		m_monitorData = _monitorData;
	}

	common::CMonitorData getMonitorData() const
	{
		return m_monitorData;
	}

	void setServicingMonitor( CPubKey const & _servicingMonitor )
	{
		m_servicingMonitor = _servicingMonitor;
	}

	CPubKey getServicingMonitor() const
	{
		return m_servicingMonitor;
	}

	~CPayLocalApplicationAction();
private:
	uint256 m_firstInitialHash;

	CTransaction m_firstTransaction;

	CTransaction m_secondTransaction;

	uintptr_t m_socket;

	CPrivKey m_privateKey;

	int64_t const m_value;

	CKeyID m_targetKey;

	std::vector<CKeyID> m_trackers;

	std::vector<CKeyID> m_monitors;

	uint256 m_validatedTransactionHash;

	common::CTrackerStats m_trackerStats;

	CPubKey m_servicingTracker;

	common::CMonitorData m_monitorData;

	CPubKey m_servicingMonitor;
};

}

#endif // PAY_LOCAL_APPLICATION_ACTION_H
