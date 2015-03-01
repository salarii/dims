// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "sendInfoRequestAction.h"

#include "serialize.h"
#include "common/support.h"
#include "common/setResponseVisitor.h"
#include "clientFilters.h"

#include <boost/assign/list_of.hpp>

namespace client
{

std::vector< TrackerInfo::Enum > const TrackerDescription = boost::assign::list_of< TrackerInfo::Enum >( TrackerInfo::Ip);//.convert_to_container<std::vector< TrackerInfo::Enum > >();

CSendInfoRequestAction::CSendInfoRequestAction( NetworkInfo::Enum const _networkInfo )
{
}

void
CSendInfoRequestAction::accept( common::CSetResponseVisitor< ClientResponses > & _visitor )
{
	_visitor.visit( *this );
}

CTrackersInfoRequest::CTrackersInfoRequest( common::CMediumFilter< ClientResponses > * _mediumFilter )
	: common::CRequest< ClientResponses >( _mediumFilter )
{
}

void
CTrackersInfoRequest::accept( common::CMedium< ClientResponses > * _medium ) const
{
	_medium->add( this );
}

/*
void
CTrackersInfoRequest::serialize( CBufferAsStream & _bufferStream ) const
{
	BOOST_FOREACH( TrackerInfo::Enum const info, m_reqInfo )
	{
		int infoCode = info;
		_bufferStream << infoCode;
	}
	
}
*/
common::CMediumFilter< ClientResponses > *
CTrackersInfoRequest::getMediumFilter() const
{
	return common::CRequest< ClientResponses >::m_mediumFilter;
}


CMonitorInfoRequest::CMonitorInfoRequest( common::CMediumFilter< ClientResponses > * _mediumFilter )
	: common::CRequest< ClientResponses >( _mediumFilter )
{
}

void
CMonitorInfoRequest::accept( common::CMedium< ClientResponses > * _medium ) const
{
	_medium->add( this );
}

}
