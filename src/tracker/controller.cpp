// Copyright (c) 2014-2015 Dims dev-team
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

unsigned int
CTrackerController::getPrice() const
{
	return m_price;
}

void
CTrackerController::setPrice( unsigned int _price )
{
	m_price = _price;
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
	return true;
}

CTrackerController::CTrackerController()
	: m_price(1000)
	, m_connected( false )
	, m_deviation(0.001)
{
	initiate();
}

}
