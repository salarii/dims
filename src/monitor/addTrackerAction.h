// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ADD_TRACKER_ACTION_H
#define ADD_TRACKER_ACTION_H

#include "configureMonitorActionHandler.h"
#include "common/action.h"
#include "common/communicationProtocol.h"

#include <boost/statechart/state_machine.hpp>
#include <vector>

namespace monitor
{

struct CUninitiated;

class CAddTrackerAction : public common::CAction< MonitorResponses >, public  boost::statechart::state_machine< CAddTrackerAction, CUninitiated >, public common::CCommunicationAction
{
public:
	//CAddTrackerAction( std::string const & _trackerAddress );

	CAddTrackerAction( std::vector< unsigned char > const & _payload, unsigned int _mediumKind );

	void setRequest( common::CRequest< MonitorResponses >* _request );

	virtual common::CRequest< MonitorResponses >* execute();

	virtual void accept( common::CSetResponseVisitor< MonitorResponses > & _visitor );

	std::vector< unsigned char > getPayload() const;


	void setMediumKind( long long unsigned _mediumKind );
// not safe
	long long unsigned getMediumKind() const;
private:
	common::CRequest< MonitorResponses >* m_request;

	std::vector< unsigned char > m_payload;

	long long unsigned m_mediumKind;
};

}

#endif // ADD_TRACKER_ACTION_H
