// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "getBalanceRequest.h"

namespace tracker
{

void CGetBalanceRequest::accept( common::CMedium< TrackerResponses > * _medium ) const
{

}

int
CGetBalanceRequest::getKind()
{
	return 0;
}

}
