#include "connectAction"

namespace node
{

CConnectAction::CConnectAction()
:m_state( State::Manual )
{

}

void
CConnectAction::accept( CSetResponseVisitor & _visitor )
{

}

CRequest*
CConnectAction::execute()
{
	if ( m_actionStatus == ActionStatus::InProgress )
	{
			
	}

	if ( State::Manual )
	{
		if (  )


		return new CTrackersInfoRequest( boost::assign::list_of<std::vector<TrackerInfo::Enum>>( TrackerInfo::Ip) );
	}
	else if (  )
}

}



