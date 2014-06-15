// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ACCEPT_NODE_ACTION_H
#define ACCEPT_NODE_ACTION_H

#include "common/action.h"
#include "configureSeedActionHandler.h"
#include <boost/statechart/state_machine.hpp>
#include <boost/optional.hpp>

namespace seed
{

class CConnectTrackerAction : public common::CAction< SeedResponses >, public  boost::statechart::state_machine< CConnectTrackerAction, CUninitiated >
{
public:
	CConnectTrackerAction( std::string const & _trackerAddress );

	CConnectTrackerAction( std::vector< unsigned char > const & _payload, unsigned int _mediumKind );

	virtual common::CRequest< TrackerResponses >* execute();

	virtual void accept( common::CSetResponseVisitor< TrackerResponses > & _visitor );

	void setRequest( common::CRequest< TrackerResponses >* _request );

	std::string getAddress() const;

	std::vector< unsigned char > getPayload() const;

	void setMediumKind( unsigned int _mediumKind );
// not safe
	unsigned int getMediumKind() const;
private:
	common::CRequest< TrackerResponses >* m_request;
};


}

#endif // ACCEPT_NODE_ACTION_H
