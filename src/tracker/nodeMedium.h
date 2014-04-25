// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NODE_MEDIUM_H
#define NODE_MEDIUM_H

#include "common/medium.h"

namespace tracker
{

class CNodeMedium : public common::CMedium
{
public:
	bool serviced() const;
	bool flush();
	bool getResponse( std::vector< RequestResponse > & _requestResponse ) const;
	void add( CRequest const * _request );
};


bool
CNodeMedium::serviced() const
{


}

bool
CNodeMedium::flush()
{

}

bool
CNodeMedium::getResponse( std::vector< RequestResponse > & _requestResponse ) const
{

}

void
CNodeMedium::add( CRequest const * _request )
{

}


}

#endif // NODE_MEDIUM_H
