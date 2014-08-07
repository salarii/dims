
#include "sendInfoRequestAction.h"

#include "serialize.h"
#include "common/support.h"
#include "common/setResponseVisitor.h"

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

CTrackersInfoRequest::CTrackersInfoRequest( std::vector< TrackerInfo::Enum > const & _reqInfo, int _mediumKind )
	: m_reqInfo( _reqInfo )
	, m_mediumKind( _mediumKind )
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
CMediumFilter< TrackerResponses > *
CTrackersInfoRequest::getMediumFilter() const
{
	return m_mediumKind;
}


CMonitorInfoRequest::CMonitorInfoRequest()
{

}

void
CMonitorInfoRequest::serialize( CBufferAsStream & _bufferStream ) const
{

}

int CMonitorInfoRequest::getMediumFilter() const
{
	return 0;
}

CInfoRequestContinue::CInfoRequestContinue( uint256 const & _token, common::RequestKind::Enum const _requestKind )
: m_token( _token )
,m_requestKind( _requestKind )
{

}

void
CInfoRequestContinue::accept( common::CMedium< NodeResponses > * _medium ) const
{
	_medium->add( this );
}

int
CInfoRequestContinue::getMediumFilter() const
{
	return m_requestKind;
}



}
