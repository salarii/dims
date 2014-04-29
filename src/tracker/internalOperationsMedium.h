// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef INTERNAL_OPERATIONS_MEDIUM_H
#define INTERNAL_OPERATIONS_MEDIUM_H

#include "common/medium.h"

namespace tracker
{

class CInternalOperationsMedium : public common::CMedium< TrackerResponses >
{
public:
	virtual bool serviced() const;
	virtual bool flush(){ return true; }
	virtual bool getResponse( std::vector< TrackerResponses > & _requestResponse ) const{ return true; }
	virtual void add( common::CRequest< TrackerResponses > const * _request ){};

};


}



#endif // INTERNAL_OPERATIONS_MEDIUM_H
