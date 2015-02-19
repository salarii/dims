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
#include "common/requestResponse.h"
#include "errorRespond.h"
#include "configureNodeActionHadler.h"

#include <boost/statechart/state_machine.hpp>

namespace client
{

struct CClientUnconnected;

class CConnectAction : public common::CAction< NodeResponses >, public  boost::statechart::state_machine< CConnectAction, CClientUnconnected >, public common::CCommunicationAction
{
public:
	CConnectAction( bool _autoDelete = true );

	virtual void accept( common::CSetResponseVisitor< NodeResponses > & _visitor );

	common::CRequest< NodeResponses >* execute();

	common::ActionStatus::Enum state();

	void setRequest( common::CRequest< NodeResponses >* _request );

	void reset();

	bool isRequestReady() const;
private:
	common::CRequest< NodeResponses >* m_request;
};

}

#endif // SEND_INFO_REQUEST_ACTION_H
