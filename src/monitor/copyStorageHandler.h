// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef COPY_STORAGE_HANDLER_H
#define COPY_STORAGE_HANDLER_H

#include "util.h"

#include "common/segmentFileStorage.h"

namespace monitor
{

class CCopyStorageHandler
{
public:
	static CCopyStorageHandler* getInstance();

	uint64_t getTimeStamp();

	common::CDiskBlock * getDiscBlock( uint64_t _id ) const;
	common::CSegmentHeader * getSegmentHeader( uint64_t _id ) const;

	uint64_t getDiscBlockSize() const;
	uint64_t getSegmentHeaderSize() const;

	void loop();

	bool createCopyRequest();

	bool copyCreated();
private:
	CCopyStorageHandler();

private:
	mutable boost::mutex m_requestLock;
	bool m_copyRequest;

	uint m_storageSize;

	uint m_headerSize;

	uint64_t m_timeStamp;

	bool m_copyCreated;

	static CCopyStorageHandler * ms_instance;
};

}

#endif // COPY_STORAGE_HANDLER_H
