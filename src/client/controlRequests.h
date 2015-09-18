// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CONTROL_REQUESTS_H
#define CONTROL_REQUESTS_H

#include "common/request.h"

namespace client
{

class CDnsInfoRequest : public common::CRequest
{
public:
	CDnsInfoRequest();

	virtual void accept( common::CMedium * _medium ) const;
};

}

#endif // CONTROL_REQUESTS_H
