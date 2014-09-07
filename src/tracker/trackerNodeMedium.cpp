// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/medium.h"
#include "trackerNodeMedium.h"
#include "common/actionHandler.h"
#include "trackerNodesManager.h"
#include "connectNodeAction.h"
#include "synchronizationRequests.h"

namespace tracker
{

CTrackerMessage::CTrackerMessage( CSynchronizationBlock const & _synchronizationInfo )
{
	m_header = common::CHeader( (int)common::CPayloadKind::SynchronizationBlock, std::vector<unsigned char>(), GetTime(), CPubKey() );

	common::createPayload( _synchronizationInfo, m_payload );

	common::CommunicationProtocol::signPayload( m_payload, m_header.m_signedHash );
}

CTrackerMessage::CTrackerMessage( CSynchronizationSegmentHeader const & _synchronizationSegmentHeader )
{
	m_header = common::CHeader( (int)common::CPayloadKind::SynchronizationHeader, std::vector<unsigned char>(), GetTime(), CPubKey() );

	common::createPayload( _synchronizationSegmentHeader, m_payload );

	common::CommunicationProtocol::signPayload( m_payload, m_header.m_signedHash );
}

void
CTrackerNodeMedium::add( CGetSynchronizationInfoRequest const * _request )
{
	common::CSynchronizationInfo synchronizationInfo;

	synchronizationInfo.m_actionKey = _request->getActionKey();

	synchronizationInfo.m_timeStamp = _request->getTimeStamp();

	common::CMessage message( synchronizationInfo );

	m_messages.push_back( message );

	m_indexes.push_back( synchronizationInfo.m_actionKey );
}

void
CTrackerNodeMedium::add( CGetNextBlockRequest const * _request )
{
	common::CGet get;

	get.m_actionKey = _request->getActionKey();

	get.m_type = _request->getBlockKind();

	common::CMessage message( get );

	m_messages.push_back( message );

	m_indexes.push_back( get.m_actionKey );
}

void
CTrackerNodeMedium::add( CSetNextBlockRequest< CSegmentHeader > const * _request )
{
	CSynchronizationSegmentHeader synchronizationSegmentHeader( _request->getBlock(), _request->getBlockIndex() );

	synchronizationSegmentHeader.m_actionKey = _request->getActionKey();

	CTrackerMessage message( synchronizationSegmentHeader );

	m_messages.push_back( message );

	m_indexes.push_back( synchronizationSegmentHeader.m_actionKey );
}

void
CTrackerNodeMedium::add( CSetNextBlockRequest< CDiskBlock > const * _request )
{
	CSynchronizationBlock synchronizationBlock( _request->getBlock(), _request->getBlockIndex() );

	synchronizationBlock.m_actionKey = _request->getActionKey();

	CTrackerMessage message( synchronizationBlock );

	m_messages.push_back( message );

	m_indexes.push_back( synchronizationBlock.m_actionKey );
}

}
