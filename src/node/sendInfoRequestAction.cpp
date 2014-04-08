
#include "sendInfoRequestAction.h"

#include "serialize.h"

namespace node
{


CSendInfoRequestAction::CSendInfoRequestAction( NetworkInfo::Enum const _networkInfo )
{
	if ( _networkInfo == NetworkInfo::Tracker )
	{
		m_request = new 
	}
/*
	

	m_actionHandler

*/	
}

void
CSendInfoRequestAction::accept( CSetResponseVisitor & _visitor )
{
	
}

CRequest*
CSendInfoRequestAction::execute()
{
	if ( m_status == ActionStatus::Unprepared )
	{
		if ( m_networkInfo )
		{

		}
	}
	else if( m_status == ActionStatus::InProgress )
	{

	}



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

CInfoRequestContinue::CInfoRequestContinue( uint256 & const _token )
{

}

void
CInfoRequestContinue::serialize( CBufferAsStream & _bufferStream ) const
{

}

RequestKind::Enum
getKind() const
{

}

}