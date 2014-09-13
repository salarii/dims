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
	CSynchronizationBlock( CDiskBlock * _diskBlock, unsigned int _blockIndex ):m_diskBlock( _diskBlock ), m_blockIndex(_blockIndex){}

	IMPLEMENT_SERIALIZE
	(
		READWRITE(*m_diskBlock);
		READWRITE(m_blockIndex);
	)

	CDiskBlock * m_diskBlock;
	unsigned int m_blockIndex;
};

struct CSynchronizationSegmentHeader
{
	CSynchronizationSegmentHeader( CSegmentHeader * _segmentHeader, unsigned int _blockIndex ):m_segmentHeader( _segmentHeader ), m_blockIndex(_blockIndex){}

	IMPLEMENT_SERIALIZE
	(
		READWRITE(*m_segmentHeader);
		READWRITE(m_blockIndex);
	)

	CSegmentHeader * m_segmentHeader;
	unsigned int m_blockIndex;
};

struct CTrackerMessage : public common::CMessage
{
public:
	CTrackerMessage( CSynchronizationBlock const & _synchronizationBlock, uint256 const & _actionKey );

	CTrackerMessage( CSynchronizationSegmentHeader const & _synchronizationSegmentHeader, uint256 const & _actionKey );
};

typedef boost::variant< common::CIdentifyMessage > ProtocolMessage;

class CGetSynchronizationInfoRequest;

class CTrackerNodeMedium : public common::CNodeMedium< TrackerResponses >
{
public:
	CTrackerNodeMedium( common::CSelfNode * _selfNode ):common::CNodeMedium< TrackerResponses >( _selfNode ){};

	void add( CGetSynchronizationInfoRequest const * _request );

	void add( CGetNextBlockRequest const * _request );

	void add( CSetNextBlockRequest< CSegmentHeader > const * _request );

	void add( CSetNextBlockRequest< CDiskBlock > const * _request );

	void add( CTransactionsPropagationRequest const * _request );

	void add( CTransactionsStatusRequest const * _request );

	void add( CPassMessageRequest const * _request );
private:
};

}

#endif // TRACKER_NODE_MEDIUM_H
