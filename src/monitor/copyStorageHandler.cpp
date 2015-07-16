// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "monitor/copyStorageHandler.h"

namespace monitor
{

CCopyStorageHandler * CCopyStorageHandler::ms_instance = NULL;

CCopyStorageHandler*
CCopyStorageHandler::getInstance()
{
	if ( !ms_instance )
	{
		ms_instance = new CCopyStorageHandler();
	};
	return ms_instance;
}

CCopyStorageHandler::CCopyStorageHandler()
{}

uint64_t
CCopyStorageHandler::getDiscBlockSize() const
{}

uint64_t
CCopyStorageHandler::getSegmentHeaderSize() const
{}


void
CCopyStorageHandler::loop()
{

	while(1)
	{
		MilliSleep(1000);
	}
}

}
