// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MONITOR_CONTROLLER_H
#define MONITOR_CONTROLLER_H

#include <boost/statechart/state_machine.hpp>

#include "uint256.h"

namespace monitor
{

struct CSynchronizeWithBitcoin;
struct CMonitorStandAlone;

// this  is utter crap and need  to  be improved  somehow

class CController : public boost::statechart::state_machine< CController, CMonitorStandAlone >
{
public:
	static CController* getInstance();

	uint64_t getTryPeriod() const
	{
		return m_tryPeriod;
	}

	uint64_t getPeriod() const
	{
		return m_period;
	}
	unsigned int getPrice() const
	{
		return m_price;
	}

	unsigned int getEnterancePrice() const
	{
		return m_enterancePrice;
	}

	void setEnterancePrice( unsigned int _price )
	{
		m_enterancePrice = _price;
	}

	void setPeriod( uint64_t const _period )
	{
		m_period = _period;
	}

	void setPrice( unsigned int _price )
	{
		m_price = _price;
	}

	void setStatusMessage( std::string const & _statusMessage )
	{
		m_statusMessage = _statusMessage;
	}

	std::string getStatusMessage()
	{
		return m_statusMessage;
	}

	bool isAdmitted() const
	{
		return m_admitted;
	}

	void setAdmitted( bool _admitted )
	{
		m_admitted = _admitted;
	}

private:
	CController();

private:
	std::string m_statusMessage;

	static CController * ms_instance;

	unsigned int m_enterancePrice;

	unsigned int m_price;

	uint64_t m_period;

	uint64_t m_tryPeriod;

	bool m_admitted;// if true  means  that monitor considers itself  as  being the valid member of network
};

}

#endif // MONITOR_CONTROLLER_H
