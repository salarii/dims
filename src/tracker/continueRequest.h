// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CONTINUE_REQUEST_H
#define CONTINUE_REQUEST_H

#include "configureTrackerActionHandler.h"
#include "common/request.h"

namespace tracker
{

class CContinueReqest : public common::CRequest< TrackerResponses >
{
public:
	CContinueReqest( uint256 const & _id, unsigned int _kind );

	void accept( common::CMedium< TrackerResponses > * _medium ) const;

	int getKind() const;

	uint256 getRequestId()const;
private:

	uint256 const m_id;
	unsigned int m_kind;
};


}

#endif // CONTINUE_REQUEST_H
