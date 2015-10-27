// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SYNCHRONIZATION_ACTION_H
#define SYNCHRONIZATION_ACTION_H

#include <boost/statechart/state_machine.hpp>

#include "common/scheduleAbleAction.h"

namespace tracker
{

struct CUninitiated;

class CSynchronizationAction : public common::CScheduleAbleAction, public  boost::statechart::state_machine< CSynchronizationAction, CUninitiated >
{
public:
	CSynchronizationAction( CPubKey const & _partnerKey );

	CSynchronizationAction( uint256 const & _actionKey, CPubKey const & _partnerKey, uint64_t _timeStamp );

	~CSynchronizationAction()
	{}

	virtual void accept( common::CSetResponseVisitor & _visitor );

	void clear();

	CPubKey getPartnerKey()const
	{
		return m_partnerKey;
	}

	bool isRequestInitialized() const;

	unsigned int getStorageSize() const { return m_storageSize; }

	void setStorageSize( unsigned int _storageSize ){ m_storageSize = _storageSize; }

	unsigned int getHeaderSize() const { return m_headerSize; }

	void setHeaderSize( unsigned int _headerSize ){ m_headerSize = _headerSize; }
private:
	uint256 m_currentHash;

	uint64_t m_timeStamp;

	CPubKey m_partnerKey;

	unsigned int m_storageSize;

	unsigned int m_headerSize;
};

}

#endif // SYNCHRONIZATION_ACTION_H
