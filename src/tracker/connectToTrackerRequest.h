// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CONNECT_TO_TRACKER_REQUEST_H
#define CONNECT_TO_TRACKER_REQUEST_H

#include "common/request.h"
#include "configureTrackerActionHandler.h"

namespace tracker
{

class CConnectToTrackerRequest : public common::CRequest< TrackerResponses >
{
public:
	CConnectToTrackerRequest( std::string const & _trackerAddress );

	virtual void accept( common::CMedium< TrackerResponses > * _medium ) const;

	virtual int getKind() const;

	std::string getAddress() const;
private:
	std::string const m_trackerAddress;

};


}

#endif // CONNECT_TO_TRACKER_REQUEST_H
