// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CONNECT_ACTION_H
#define CONNECT_ACTION_H

#include <boost/optional.hpp>

#include "action.h"
#include "uint256.h"

#include "trackerLocalRanking.h"
#include "sendInfoRequestAction.h"
#include "requestRespond.h"
#include "errorRespond.h"



namespace node
{

struct CConnectActionState;

class CConnectAction : public CAction
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

	void accept( CSetResponseVisitor & _visitor );

	CRequest* execute();

	ActionStatus::Enum state();

	void reset();

	void setTrackerInfo( boost::optional< CTrackerStats > const & _trackerInfo );

	void setInProgressToken( boost::optional< uint256 > const & _token );

    void setMediumError( boost::optional< ErrorType::Enum > const & _error );
private:
	State::Enum m_state;

	boost::optional< uint256 > m_token;

	boost::optional< CTrackerStats > m_trackerStats;

	boost::optional< std::vector< std::string > > m_monitorInfo;

    boost::optional< ErrorType::Enum > m_error;

	CConnectActionState * m_actionState;
};

}

#endif // SEND_INFO_REQUEST_ACTION_H
