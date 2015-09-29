// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MESSAGE_TYPE_H
#define MESSAGE_TYPE_H

#include <QString>

namespace dims
{

QString const ServerName = "/tmp/dimsClient";

struct CMessageKind
{
	enum Enum
	{
		  Expectations
		, ErrorIndicator
		, Transaction
	};
};

struct CAppError
{
	enum Enum
	{
		  RefusedByClient
		, NotEnoughFunds
		, DifferentNetwork
	};
};


}
#endif // MESSAGE_TYPE_H
