
#include "sendInfoRequestAction.h"

#include "serialize.h"
#include "common/support.h"
#include "common/setResponseVisitor.h"

#include <boost/assign/list_of.hpp>

namespace node
{

std::vector< TrackerInfo::Enum > const TrackerDescription = boost::assign::list_of< TrackerInfo::Enum >( TrackerInfo::Ip)( TrackerInfo::Port);//.convert_to_container<std::vector< TrackerInfo::Enum > >();

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

CTrackersInfoRequest::CTrackersInfoRequest( std::vector< TrackerInfo::Enum > const & _reqInfo )
: m_reqInfo( _reqInfo )
{
}

void
CTrackersInfoRequest::serialize( CBufferAsStream & _bufferStream ) const
{
	BOOST_FOREACH( TrackerInfo::Enum const info, m_reqInfo )
	{
		int infoCode = info;
		_bufferStream << infoCode;
	}
	
}

int
CTrackersInfoRequest::getKind() const
{
	return common::RequestKind::NetworkInfo;
}


CMonitorInfoRequest::CMonitorInfoRequest()
{

}

void
CMonitorInfoRequest::serialize( CBufferAsStream & _bufferStream ) const
{

}

int CMonitorInfoRequest::getKind() const
{
	return 0;
}

CInfoRequestContinue::CInfoRequestContinue( uint256 const & _token, common::RequestKind::Enum const _requestKind )
: m_token( _token )
,m_requestKind( _requestKind )
{

}

void
CInfoRequestContinue::serialize( CBufferAsStream & _bufferStream ) const
{
	common::serializeEnum( _bufferStream, common::CMainRequestType::ContinueReq );
	_bufferStream << m_token;
}

int
CInfoRequestContinue::getKind() const
{
	return m_requestKind;
}



}
