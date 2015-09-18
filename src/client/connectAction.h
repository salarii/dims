// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CONNECT_ACTION_H
#define CONNECT_ACTION_H

#include <boost/optional.hpp>

#include "common/action.h"
#include "uint256.h"

#include "trackerLocalRanking.h"
#include "sendInfoRequestAction.h"
#include "common/responses.h"
#include "errorRespond.h"

#include <boost/statechart/state_machine.hpp>

namespace client
{

struct CClientUnconnected;

class CConnectAction : public common::CAction, public  boost::statechart::state_machine< CConnectAction, CClientUnconnected >
{
public:
	CConnectAction( bool _autoDelete = true );

	virtual void accept( common::CSetResponseVisitor & _visitor );

	void reset();

	bool isRequestReady() const;

	//experimental
	boost::signals2::signal< void () > m_connected;
};

}

#endif // SEND_INFO_REQUEST_ACTION_H
