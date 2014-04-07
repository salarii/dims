
#include "sendInfoRequestAction.h"

#include "serialize.h"

namespace node
{

CTrackersInfoRequest::CTrackersInfoRequest( std::vector< TrackerInfo::Enum > & const _reqInfo )
: m_reqInfo( _reqInfo )
{
}

void
CTrackersInfoRequest::serialize( CBufferAsStream & _bufferStream ) const
{
	_bufferStream << m_reqInfo;
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



}