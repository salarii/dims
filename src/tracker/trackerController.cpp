// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "trackerController.h"

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

CTrackerController::CTrackerController()
{
}

}
