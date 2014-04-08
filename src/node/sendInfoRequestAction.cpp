
#include "sendInfoRequestAction.h"

#include "serialize.h"
#include "support.h"

namespace node
{


CSendInfoRequestAction::CSendInfoRequestAction( NetworkInfo::Enum const _networkInfo )
{
}

void
CSendInfoRequestAction::accept( CSetResponseVisitor & _visitor )
{
	
}

CRequest*
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

RequestKind::Enum
CTrackersInfoRequest::getKind() const
{
	return RequestKind::NetworkInfo;
}


CMonitorInfoRequest::CMonitorInfoRequest()
{

}

void
CMonitorInfoRequest::serialize( CBufferAsStream & _bufferStream ) const
{

}

RequestKind::Enum
CMonitorInfoRequest::getKind() const
{

}

CInfoRequestContinue::CInfoRequestContinue( uint256 const & _token, RequestKind::Enum const _requestKind )
: m_token( _token )
,m_requestKind( _requestKind )
{

}

void
CInfoRequestContinue::serialize( CBufferAsStream & _bufferStream ) const
{

	serializeEnum( _bufferStream, m_requestKind );
	_bufferStream << m_token;
}

RequestKind::Enum
CInfoRequestContinue::getKind() const
{
	return m_requestKind;
}

}