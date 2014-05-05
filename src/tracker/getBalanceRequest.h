// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef GET_BALANCE_REQUEST_H
#define GET_BALANCE_REQUEST_H

#include "common/request.h"
#include "configureTrackerActionHandler.h"

#include "uint256.h"

namespace tracker
{

class CGetBalanceRequest : public common::CRequest< TrackerResponses >
{
public:
	CGetBalanceRequest( uint160 const & _key );

	virtual void accept( common::CMedium< TrackerResponses > * _medium ) const;

	virtual int getKind();

	uint160 getKey() const;
private:
	uint160 m_key;
};


}


#endif // GET_BALANCE_REQUEST_H
