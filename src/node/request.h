// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef REQUEST_H
#define REQUEST_H

struct CBufferAsStream;

namespace node
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

struct CRequest
{
	virtual void serialize( CBufferAsStream & _bufferStream ) const = 0;

	virtual RequestKind::Enum getKind() const = 0;
};

}

#endif