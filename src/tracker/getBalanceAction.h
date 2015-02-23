// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef GET_BALANCE_ACTION_H
#define GET_BALANCE_ACTION_H

#include "configureTrackerActionHandler.h"
#include "common/action.h"

namespace tracker
{

class CGetBalanceAction : public common::CAction< TrackerResponses >
{
public:
	CGetBalanceAction( uint160 const & _keyId, uint256 const & _hash );

	virtual void accept( common::CSetResponseVisitor< TrackerResponses > & _visitor );

	virtual common::CRequest< TrackerResponses >* getRequest() const;

	virtual void reset(){}

	void passBalance( common::CAvailableCoins const & _availableCoins );
private:
	common::CRequest< TrackerResponses >* m_request;

	uint160 const m_keyId;

	uint256 const m_hash;
};

}

#endif // GET_BALANCE_ACTION_H
