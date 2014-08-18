// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TRACKER_CONTROLLER_H
#define TRACKER_CONTROLLER_H

#include <boost/statechart/state_machine.hpp>

namespace tracker
{
struct CInitialSynchronization;

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
private:
	CTrackerController();

private:
	static CTrackerController * ms_instance;

	float m_price;

	unsigned int m_maxPrice;

	unsigned int m_minPrice;
};


}

#endif // TRACKER_CONTROLLER_H
