// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef REQUEST_RESPOND_H
#define REQUEST_RESPOND_H

#include <boost/variant.hpp>

namespace node
{

struct CTransactionStatus;
struct CAccountBalance;

typedef boost::variant< CTransactionStatus, CAccountBalance > RequestRespond;

}

#endif