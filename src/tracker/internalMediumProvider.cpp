// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "internalMediumProvider.h"
#include "internalOperationsMedium.h"
#include "trackerMediumsKinds.h"

namespace tracker
{

CInternalMediumProvider::CInternalMediumProvider()
{
	m_mediumProviders.push_back( CInternalOperationsMedium::getInstance() );
}

std::list< common::CMedium< TrackerResponses > *>
CInternalMediumProvider::provideConnection( int const _actionKind, unsigned _requestedConnectionNumber )
{
	if ( CTrackerMediumsKinds::Internal == _actionKind )
		return m_mediumProviders;
	else
		return std::list< common::CMedium< TrackerResponses > *>();
}

}
