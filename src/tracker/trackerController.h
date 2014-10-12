// Copyright (c) 2014 Dims dev-team
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

	float getPrice() const;
	void setPrice( float _price );

	int getMaxPrice() const;
	void setMaxPrice( int _price );

	int getMinPrice() const;
	void setMinPrice( int _price );

	bool isConnected() const;
	bool setConnected( bool _connected );

	// monitor related
	CMonitorData & acquireMonitorData()
	{
		return m_monitorData;
	}


private:
	CTrackerController();

private:
	static CTrackerController * ms_instance;

	float m_price;

	unsigned int m_maxPrice;

	unsigned int m_minPrice;

	bool m_connected;

	CMonitorData m_monitorData;
};


}

#endif // TRACKER_CONTROLLER_H
