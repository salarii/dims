// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PROVIDE_INFO_ACTION_H
#define PROVIDE_INFO_ACTION_H

#include "common/scheduleAbleAction.h"
#include "common/filters.h"
#include "common/mediumKinds.h"

#include <boost/statechart/state_machine.hpp>

#include "protocol.h"

namespace tracker
{

struct CInit;
// rework  this  sooner  or later

class CProvideInfoAction : public common::CScheduleAbleAction, public  boost::statechart::state_machine< CProvideInfoAction, CInit >
{
public:
	CProvideInfoAction( uint256 const & _actionKey, CPubKey const & _partnerKey );

	CProvideInfoAction( common::CInfoKind::Enum _infoKind, CPubKey const & _partnerKey );

	CProvideInfoAction( common::CInfoKind::Enum _infoKind, common::CMediumKinds::Enum _mediumKind );

	virtual void accept( common::CSetResponseVisitor & _visitor );

	CPubKey getPartnerKey()const
	{
		return m_partnerKey;
	}

	common::CInfoKind::Enum getInfo() const{ return m_infoKind; }

	~CProvideInfoAction(){};
private:
	common::CInfoKind::Enum m_infoKind;

	CPubKey m_partnerKey;
};

}

#endif // PROVIDE_INFO_ACTION_H
