// Copyright (c) 2014 Ratcoin dev-team
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

namespace node
{

struct CConnectActionState;

class CConnectAction : public common::CAction< NodeResponses >
{
public:
	struct State
	{
		enum Enum
		{
		  Manual
		, KnownSeed
		, KnownMonitor
		, KnownTracker
		, Done
		};
	};
public:
	CConnectAction( State::Enum const _state );

	virtual void accept( common::CSetResponseVisitor< NodeResponses > & _visitor );

	common::CRequest< NodeResponses >* execute();

	common::ActionStatus::Enum state();

	void reset();

	void setTrackerInfo( boost::optional< common::CTrackerStats > const & _trackerInfo );

	void setInProgressToken( boost::optional< uint256 > const & _token );

	void setMediumError( boost::optional< common::ErrorType::Enum > const & _error );
private:
	State::Enum m_state;

	boost::optional< uint256 > m_token;

	boost::optional< common::CTrackerStats > m_trackerStats;

	boost::optional< std::vector< std::string > > m_monitorInfo;

	boost::optional< common::ErrorType::Enum > m_error;
};

}

#endif // SEND_INFO_REQUEST_ACTION_H
