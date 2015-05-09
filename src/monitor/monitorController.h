// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MONITOR_CONTROLLER_H
#define MONITOR_CONTROLLER_H

#include <boost/statechart/state_machine.hpp>

#include "uint256.h"

namespace monitor
{

struct CSynchronizeWithBitcoin;

class CMonitorController : public boost::statechart::state_machine< CMonitorController, CSynchronizeWithBitcoin >
{
public:
	static CMonitorController* getInstance();

	uint64_t getPeriod() const
	{
		return m_period;
	}
	unsigned int getPrice() const
	{
		return m_price;
	}

	void setPeriod( uint64_t const _period )
	{
		m_period = _period;
	}

	void setPrice( unsigned int _price )
	{
		m_price = _price;
	}
private:
	CMonitorController();

private:
	static CMonitorController * ms_instance;

	unsigned int m_price;
	uint64_t m_period;
};

}

#endif // MONITOR_CONTROLLER_H
