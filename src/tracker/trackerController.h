// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TRACKER_CONTROLLER_H
#define TRACKER_CONTROLLER_H

#include <boost/statechart/state_machine.hpp>
#include "key.h"

namespace tracker
{
struct CInitialSynchronization;

struct CMonitorData
{
	CMonitorData( bool _isAdmitted = false, CPubKey _monitorPublicKey = CPubKey(), bool _allowAdmission = true, double _accepableRatio = 0 ):m_isAdmitted( m_isAdmitted ), m_monitorPublicKey( _monitorPublicKey ), m_allowAdmission( _allowAdmission ), m_accepableRatio( _accepableRatio ){}
	bool m_isAdmitted;
	CPubKey m_monitorPublicKey;
	bool m_allowAdmission;
	double m_accepableRatio;// price / period
};

class CTrackerController : public boost::statechart::state_machine< CTrackerController, CInitialSynchronization >
{
public:
	static CTrackerController* getInstance();

	unsigned int getPrice() const;
	void setPrice( unsigned int _price );

	bool isConnected() const;
	bool setConnected( bool _connected );

	// monitor related
	CMonitorData & acquireMonitorData()
	{
		return m_monitorData;
	}

	void setStatusMessage( std::string const & _statusMessage )
	{
		m_statusMessage = _statusMessage;
	}

	std::string getStatusMessage()
	{
		return m_statusMessage;
	}

private:
	CTrackerController();

private:
	static CTrackerController * ms_instance;

	unsigned int m_price;

	bool m_connected;

	CMonitorData m_monitorData;

	float const m_deviation;

	std::string m_statusMessage;
};


}

#endif // TRACKER_CONTROLLER_H
