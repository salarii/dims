// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/segmentFileStorage.h"

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

bool
CCopyStorageHandler::createCopyRequest()
{
	boost::lock_guard<boost::mutex> lock( m_requestLock );
	if ( m_copyRequest )
		return false;

	m_copyRequest = true;

	return true;
}

bool
CCopyStorageHandler::copyCreated()
{
	return m_copyCreated;
}

CCopyStorageHandler::CCopyStorageHandler()
	: m_copyRequest( false )
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
		if ( m_copyRequest )
		{
			m_copyCreated = false;

			m_storageSize = common::CSegmentFileStorage::getInstance()->calculateStoredBlockNumber();
			m_headerSize = common::CSegmentFileStorage::getInstance()->getStoredHeaderCount();

			common::CSegmentFileStorage::getInstance()->copyHeader();
			common::CSegmentFileStorage::getInstance()->copyStorage();

			m_copyCreated = true;
			m_copyRequest = false;
		}

		MilliSleep(1000);
	}
}

}
