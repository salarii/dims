// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TRANSACTION_STATUS_H
#define TRANSACTION_STATUS_H

namespace common
{

struct TransactionsStatus
{
	enum Enum
	{
		  Invalid = 0
		, Valdated
		, Unconfirmed
		, Ack
		, DoubleSpend
		, Confirmed
		, Investigated
	};
};

}

#endif // RATCOIN_PARAMS_H
