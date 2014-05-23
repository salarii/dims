// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CONNECT_TRACKER_ACTION_H
#define CONNECT_TRACKER_ACTION_H

#include "common/action.h"
#include "configureTrackerActionHandler.h"
#include <boost/statechart/state_machine.hpp>
#include <boost/optional.hpp>

namespace tracker
{

struct CUninitiated;

class CConnectTrackerAction : public common::CAction< TrackerResponses >, public  boost::statechart::state_machine< CConnectTrackerAction, CUninitiated >
{
public:
	CConnectTrackerAction( std::string const & _trackerAddress );

	CConnectTrackerAction( std::vector< unsigned char > const & _payload );

	virtual common::CRequest< TrackerResponses >* execute();

	virtual void accept( common::CSetResponseVisitor< TrackerResponses > & _visitor );

	void setRequest( common::CRequest< TrackerResponses >* _request );

	std::string getAddress() const;

	std::vector< unsigned char > getPayload() const;
// not safe
	unsigned int getMediumKind() const;
private:
	common::CRequest< TrackerResponses >* m_request;
	std::string const m_trackerAddress;

	static int const ms_randomPayloadLenght = 32;

	std::vector< unsigned char > m_payload;

	unsigned int m_mediumKind;

	bool const m_passive;
};


}

#endif // CONNECT_TRACKER_ACTION_H
