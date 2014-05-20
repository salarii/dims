// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NODE_MEDIUM_H
#define NODE_MEDIUM_H

#include "common/medium.h"

namespace tracker
{

class CIdentifyRequest;

class CNodeMedium : public common::CMedium< TrackerResponses >
{
public:
	bool serviced() const;
	bool flush();
	bool getResponse( std::vector< TrackerResponses > & _requestResponse ) const;
	void add( common::CRequest< TrackerResponses > const * _request );

	void add( CIdentifyRequest const * _request );

};


}

#endif // NODE_MEDIUM_H
