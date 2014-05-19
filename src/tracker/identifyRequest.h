// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef IDENTIFY_REQUEST_H
#define IDENTIFY_REQUEST_H

#include "common/request.h"
#include "net.h"
#include "configureTrackerActionHandler.h"

namespace tracker
{

class CIdentifyRequest : public common::CRequest< TrackerResponses >
{
public:
	CIdentifyRequest( CNode * _node );

	void accept( common::CMedium< TrackerResponses > * _medium ) const;

	int getKind() const;
private:
	static int const ms_randomPayloadLenght = 32;

	unsigned char m_payload[ ms_randomPayloadLenght ];
};

}

#endif // IDENTIFY_REQUEST_H
