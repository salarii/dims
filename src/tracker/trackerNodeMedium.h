// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TRACKER_NODE_MEDIUM_H
#define TRACKER_NODE_MEDIUM_H

#include "common/nodeMedium.h"
#include "common/communicationProtocol.h"

#include "segmentFileStorage.h"

#include <boost/variant.hpp>

namespace tracker
{
// not good place  to put  this
struct CSynchronizationBlock
{
	CSynchronizationBlock( CDiskBlock * _diskBlock ):m_diskBlock( _diskBlock ){}

	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_actionKey);
		READWRITE(*m_diskBlock);
	)

	uint256 m_actionKey;
	CDiskBlock * m_diskBlock;
};

struct CTrackerMessage : public common::CMessage
{
public:
	CTrackerMessage( CSynchronizationBlock const & _synchronizationBlock );
};

typedef boost::variant< common::CIdentifyMessage > ProtocolMessage;

class CGetSynchronizationInfoRequest;

class CTrackerNodeMedium : public common::CNodeMedium< TrackerResponses >
{
public:
	CTrackerNodeMedium( common::CSelfNode * _selfNode ):common::CNodeMedium< TrackerResponses >( _selfNode ){};

	void add( CGetSynchronizationInfoRequest const * _request );

	void add( CGetNextBlockRequest const * _request );

	void add( CSetNextBlockRequest const * _request );
private:
};

}

#endif // TRACKER_NODE_MEDIUM_H
