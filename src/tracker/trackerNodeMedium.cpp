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
	common::CSynchronizationGetBlock synchronizationGetBlock;

	common::CMessage message( synchronizationGetBlock );

	m_messages.push_back( message );

	m_indexes.push_back( synchronizationGetBlock.m_actionKey );
}

void
CTrackerNodeMedium::add( CSetNextBlockRequest const * _request )
{
	common::CSynchronizationBlock synchronizationBlock;

	common::CMessage message( synchronizationBlock );

	m_messages.push_back( message );

	m_indexes.push_back( synchronizationBlock.m_actionKey );
}

}
