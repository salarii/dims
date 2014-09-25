// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "trackerController.h"

#include "trackerControllerStates.h"

namespace tracker
{

CTrackerController * CTrackerController::ms_instance = NULL;

CTrackerController*
CTrackerController::getInstance()
{
	if ( !ms_instance )
	{
		ms_instance = new CTrackerController();
	};
	return ms_instance;
}

float
CTrackerController::getPrice() const
{
	return m_price;
}

void
CTrackerController::setPrice( float _price )
{
	m_price = _price;
}

int
CTrackerController::getMaxPrice() const
{
	return m_maxPrice;
}

void
CTrackerController::setMaxPrice( int _price )
{
	m_maxPrice = _price;
}

int
CTrackerController::getMinPrice() const
{
	return m_minPrice;
}

void
CTrackerController::setMinPrice( int _price )
{
	m_minPrice = _price;
}

bool
CTrackerController::isConnected() const
{
	return m_connected;
}

bool
CTrackerController::setConnected( bool _connected )
{
	m_connected = _connected;
}

CTrackerController::CTrackerController()
	: m_price(0.01)
	, m_maxPrice(1000000)
	, m_minPrice(1000)
{
	initiate();
}

}
