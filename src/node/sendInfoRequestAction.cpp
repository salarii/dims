// Copyright (c) 2014 Dims dev-team
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
CSendInfoRequestAction::accept( common::CSetResponseVisitor< NodeResponses > & _visitor )
{
	_visitor.visit( *this );
}

common::CRequest< NodeResponses >*
CSendInfoRequestAction::execute()
{
	return 0;
}

CTrackersInfoRequest::CTrackersInfoRequest( std::vector< TrackerInfo::Enum > const & _reqInfo, common::CMediumFilter< NodeResponses > * _mediumFilter )
	: common::CRequest< NodeResponses >( _mediumFilter )
	, m_reqInfo( _reqInfo )
{
}

void
CTrackersInfoRequest::accept( common::CMedium< NodeResponses > * _medium ) const
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
common::CMediumFilter< NodeResponses > *
CTrackersInfoRequest::getMediumFilter() const
{
	return common::CRequest< NodeResponses >::m_mediumFilter;
}


CMonitorInfoRequest::CMonitorInfoRequest()
	: common::CRequest< NodeResponses >( new CMediumClassFilter( -1 ) )
{

}

void
CMonitorInfoRequest::serialize( CBufferAsStream & _bufferStream ) const
{

}

common::CMediumFilter< NodeResponses > *
CMonitorInfoRequest::getMediumFilter() const
{
	return common::CRequest< NodeResponses >::m_mediumFilter;
}

CInfoRequestContinueComplex::CInfoRequestContinueComplex( std::map< uintptr_t, uint256 > const & _nodeToToken, common::CMediumFilter< NodeResponses > * _mediumFilter )
	: common::CRequest< NodeResponses >( _mediumFilter )
	, m_nodeToToken( _nodeToToken )
{
}

void
CInfoRequestContinueComplex::accept( common::CMedium< NodeResponses > * _medium ) const
{
	_medium->add( this );
}

common::CMediumFilter< NodeResponses > *
CInfoRequestContinueComplex::getMediumFilter() const
{
	return common::CRequest< NodeResponses >::m_mediumFilter;
}
}
