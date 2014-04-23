// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef REQUEST_H
#define REQUEST_H

struct CBufferAsStream;

namespace common
{


struct RequestKind
{
	enum Enum
	{
		NetworkInfo = 0,
		Transaction,
		TransactionStatus,
		Balance,
		Seed
	};
};

class CMedium;

class CRequestVisitor;

struct CRequest
{
	virtual void accept( CMedium * _medium ) const = 0;
// reconsider this  int
	virtual int getKind() = 0;
};


}

#endif
